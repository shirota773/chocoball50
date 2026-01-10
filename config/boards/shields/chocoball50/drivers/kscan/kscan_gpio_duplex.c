/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_kscan_gpio_duplex

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define INST_DUPLEX_LEN(n) DT_INST_PROP(n, duplex_gpios)
#define INST_MATRIX_LEN(n) DT_INST_PROP_LEN(n, gpios)
#define INST_COLS(n) INST_DUPLEX_LEN(n)
#define INST_ROWS(n) (INST_MATRIX_LEN(n) - INST_DUPLEX_LEN(n))

struct kscan_duplex_data {
    kscan_callback_t callback;
    struct k_work_delayable work;
    const struct device *dev;
    uint32_t scan_period;
    /* Current state (pass 1 + pass 2 combined) */
    uint8_t matrix_state[INST_MATRIX_LEN(0)];
    /* Previous state for change detection */
    uint8_t last_state[INST_MATRIX_LEN(0)];
};

struct kscan_duplex_config {
    struct gpio_dt_spec *gpios;
    size_t num_gpios;
    size_t duplex_index; /* Split point: [0..duplex_index) = cols in pass1, [duplex_index..end) = rows in pass1 */
    uint32_t debounce_period_ms;
    uint32_t poll_period_ms;
    bool use_interrupt;
};

/* Helper: Configure GPIO as output (driving column) */
static int gpio_set_output(const struct gpio_dt_spec *gpio) {
    return gpio_pin_configure_dt(gpio, GPIO_OUTPUT_INACTIVE);
}

/* Helper: Configure GPIO as input with pull-down (sensing row) */
static int gpio_set_input(const struct gpio_dt_spec *gpio) {
    return gpio_pin_configure_dt(gpio, GPIO_INPUT | GPIO_PULL_DOWN);
}

/* Scan one pass of the duplex matrix
 * drive_start, drive_end: indices of pins to drive (columns)
 * sense_start, sense_end: indices of pins to sense (rows)
 * state: output buffer for key states
 * state_offset: base index in state array for this pass
 */
static void scan_pass(const struct kscan_duplex_config *config,
                     uint8_t *state,
                     size_t drive_start, size_t drive_end,
                     size_t sense_start, size_t sense_end,
                     size_t state_offset) {
    size_t num_cols = drive_end - drive_start;
    size_t num_rows = sense_end - sense_start;

    /* Configure sense pins as inputs */
    for (size_t i = sense_start; i < sense_end; i++) {
        gpio_set_input(&config->gpios[i]);
    }

    /* Scan each column */
    for (size_t col = 0; col < num_cols; col++) {
        size_t col_idx = drive_start + col;

        /* Drive this column HIGH */
        gpio_pin_set_dt(&config->gpios[col_idx], 1);

        /* Small delay for signal to stabilize */
        k_busy_wait(1);

        /* Read all rows */
        for (size_t row = 0; row < num_rows; row++) {
            size_t row_idx = sense_start + row;
            int val = gpio_pin_get_dt(&config->gpios[row_idx]);

            /* Store in state array: state[row * num_cols + col] */
            size_t state_idx = state_offset + (row * num_cols + col);
            state[state_idx] = (val > 0) ? 1 : 0;
        }

        /* Drive column LOW again */
        gpio_pin_set_dt(&config->gpios[col_idx], 0);
    }

    /* Set all drive pins back to input to save power */
    for (size_t i = drive_start; i < drive_end; i++) {
        gpio_set_input(&config->gpios[i]);
    }
}

/* Main scanning work handler */
static void kscan_duplex_work_handler(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct kscan_duplex_data *data = CONTAINER_OF(dwork, struct kscan_duplex_data, work);
    const struct device *dev = data->dev;
    const struct kscan_duplex_config *config = dev->config;

    /* Clear current state */
    memset(data->matrix_state, 0, sizeof(data->matrix_state));

    /* Pass 1: First N pins as columns (output), remaining pins as rows (input)
     * This scans the COL2ROW portion of the matrix
     */
    size_t pass1_cols_start = 0;
    size_t pass1_cols_end = config->duplex_index;
    size_t pass1_rows_start = config->duplex_index;
    size_t pass1_rows_end = config->num_gpios;
    size_t pass1_num_cols = pass1_cols_end - pass1_cols_start;
    size_t pass1_num_rows = pass1_rows_end - pass1_rows_start;

    scan_pass(config, data->matrix_state,
             pass1_cols_start, pass1_cols_end,
             pass1_rows_start, pass1_rows_end,
             0);

    /* Pass 2: Swap roles - first N pins as rows (input), remaining as columns (output)
     * This scans the ROW2COL portion of the matrix
     */
    size_t pass2_cols_start = config->duplex_index;
    size_t pass2_cols_end = config->num_gpios;
    size_t pass2_rows_start = 0;
    size_t pass2_rows_end = config->duplex_index;
    size_t pass2_num_cols = pass2_cols_end - pass2_cols_start;
    size_t pass2_num_rows = pass2_rows_end - pass2_rows_start;
    size_t pass2_offset = pass1_num_rows * pass1_num_cols;

    scan_pass(config, data->matrix_state,
             pass2_cols_start, pass2_cols_end,
             pass2_rows_start, pass2_rows_end,
             pass2_offset);

    /* Detect changes and report to callback */
    bool any_change = false;
    size_t total_keys = (pass1_num_rows * pass1_num_cols) + (pass2_num_rows * pass2_num_cols);

    for (size_t i = 0; i < total_keys; i++) {
        if (data->matrix_state[i] != data->last_state[i]) {
            any_change = true;
            data->last_state[i] = data->matrix_state[i];

            /* Report key state change */
            if (data->callback != NULL) {
                /* Calculate row and column for callback
                 * For pass 1 keys (0 to pass2_offset-1):
                 *   row = i / pass1_num_cols
                 *   col = i % pass1_num_cols
                 * For pass 2 keys (pass2_offset to total_keys-1):
                 *   adjusted_i = i - pass2_offset
                 *   row = pass1_num_rows + (adjusted_i / pass2_num_cols)
                 *   col = adjusted_i % pass2_num_cols
                 */
                uint32_t row, col;
                if (i < pass2_offset) {
                    row = i / pass1_num_cols;
                    col = i % pass1_num_cols;
                } else {
                    size_t adjusted = i - pass2_offset;
                    row = pass1_num_rows + (adjusted / pass2_num_cols);
                    col = adjusted % pass2_num_cols;
                }

                data->callback(dev, row, col, data->matrix_state[i]);
            }
        }
    }

    /* Schedule next scan */
    k_work_schedule(&data->work, K_MSEC(any_change ? config->debounce_period_ms : config->poll_period_ms));
}

static int kscan_duplex_configure(const struct device *dev, kscan_callback_t callback) {
    struct kscan_duplex_data *data = dev->data;

    if (!callback) {
        return -EINVAL;
    }

    data->callback = callback;
    return 0;
}

static int kscan_duplex_enable(const struct device *dev) {
    struct kscan_duplex_data *data = dev->data;

    /* Start scanning */
    k_work_schedule(&data->work, K_NO_WAIT);
    return 0;
}

static int kscan_duplex_disable(const struct device *dev) {
    struct kscan_duplex_data *data = dev->data;

    /* Stop scanning */
    k_work_cancel_delayable(&data->work);
    return 0;
}

static int kscan_duplex_init(const struct device *dev) {
    struct kscan_duplex_data *data = dev->data;
    const struct kscan_duplex_config *config = dev->config;

    data->dev = dev;

    /* Initialize all GPIOs as inputs */
    for (size_t i = 0; i < config->num_gpios; i++) {
        if (!device_is_ready(config->gpios[i].port)) {
            LOG_ERR("GPIO port not ready for pin %d", i);
            return -ENODEV;
        }

        int ret = gpio_set_input(&config->gpios[i]);
        if (ret < 0) {
            LOG_ERR("Failed to configure GPIO %d as input: %d", i, ret);
            return ret;
        }
    }

    /* Initialize work */
    k_work_init_delayable(&data->work, kscan_duplex_work_handler);

    LOG_INF("Duplex matrix initialized: %d GPIOs, duplex split at %d",
            config->num_gpios, config->duplex_index);

    return 0;
}

static const struct kscan_driver_api kscan_duplex_api = {
    .config = kscan_duplex_configure,
    .enable_callback = kscan_duplex_enable,
    .disable_callback = kscan_duplex_disable,
};

#define KSCAN_DUPLEX_INIT(n)                                                                      \
    static struct gpio_dt_spec kscan_duplex_gpios_##n[] = {                                       \
        DT_FOREACH_PROP_ELEM_SEP(DT_DRV_INST(n), gpios, GPIO_DT_SPEC_GET_BY_IDX, (,))           \
    };                                                                                             \
                                                                                                   \
    static struct kscan_duplex_data kscan_duplex_data_##n = {                                     \
        .matrix_state = {0},                                                                       \
        .last_state = {0},                                                                         \
    };                                                                                             \
                                                                                                   \
    static const struct kscan_duplex_config kscan_duplex_config_##n = {                           \
        .gpios = kscan_duplex_gpios_##n,                                                          \
        .num_gpios = ARRAY_SIZE(kscan_duplex_gpios_##n),                                         \
        .duplex_index = DT_INST_PROP(n, duplex_gpios),                                           \
        .debounce_period_ms = DT_INST_PROP_OR(n, debounce_period_ms, 5),                        \
        .poll_period_ms = DT_INST_PROP_OR(n, poll_period_ms, 10),                               \
    };                                                                                             \
                                                                                                   \
    DEVICE_DT_INST_DEFINE(n, kscan_duplex_init, NULL,                                            \
                         &kscan_duplex_data_##n, &kscan_duplex_config_##n,                       \
                         POST_KERNEL, 81,                                                        \
                         &kscan_duplex_api);

DT_INST_FOREACH_STATUS_OKAY(KSCAN_DUPLEX_INIT)
