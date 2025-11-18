/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT palette_az1uball

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(az1uball, CONFIG_INPUT_LOG_LEVEL);

#define AZ1UBALL_I2C_ADDRESS 0x0A
#define AZ1UBALL_DATA_SIZE 5

/* az1uball mode commands */
#define AZ1UBALL_CMD_NORMAL_MODE 0x90
#define AZ1UBALL_CMD_AZ_MODE 0x91

struct az1uball_config {
	struct i2c_dt_spec i2c;
	uint8_t invert_x;
	uint8_t invert_y;
	uint8_t swap_xy;
};

struct az1uball_data {
	const struct device *dev;
	struct k_work_delayable work;
	uint8_t prev_button;
};

static void az1uball_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct az1uball_data *data = CONTAINER_OF(dwork, struct az1uball_data, work);
	const struct device *dev = data->dev;
	const struct az1uball_config *config = dev->config;

	uint8_t buf[AZ1UBALL_DATA_SIZE];
	int ret;

	/* Read 5 bytes from az1uball */
	ret = i2c_read_dt(&config->i2c, buf, sizeof(buf));
	if (ret < 0) {
		LOG_ERR("Failed to read from az1uball: %d", ret);
		goto schedule;
	}

	/* Parse movement data
	 * buf[0]: left movement
	 * buf[1]: right movement
	 * buf[2]: up movement
	 * buf[3]: down movement
	 * buf[4]: button state (0x80 = pressed, 0x00 = released)
	 */
	int16_t dx = (int16_t)buf[1] - (int16_t)buf[0];
	int16_t dy = (int16_t)buf[3] - (int16_t)buf[2];

	/* Apply inversion and swap if configured */
	if (config->invert_x) {
		dx = -dx;
	}
	if (config->invert_y) {
		dy = -dy;
	}
	if (config->swap_xy) {
		int16_t tmp = dx;
		dx = dy;
		dy = tmp;
	}

	/* Report movement if non-zero */
	if (dx != 0) {
		input_report_rel(dev, INPUT_REL_X, dx, false, K_FOREVER);
	}
	if (dy != 0) {
		input_report_rel(dev, INPUT_REL_Y, dy, false, K_FOREVER);
	}

	/* Report button state changes */
	uint8_t button = (buf[4] & 0x80) ? 1 : 0;
	if (button != data->prev_button) {
		input_report_key(dev, INPUT_BTN_0, button, true, K_FOREVER);
		data->prev_button = button;
	}

	/* Sync if any events were reported */
	if (dx != 0 || dy != 0 || button != data->prev_button) {
		input_report_abs(dev, INPUT_ABS_X, 0, false, K_FOREVER);
		input_report_abs(dev, INPUT_ABS_Y, 0, true, K_FOREVER);
	}

schedule:
	/* Schedule next poll */
	k_work_schedule(&data->work, K_MSEC(CONFIG_AZ1UBALL_POLL_INTERVAL_MS));
}

static int az1uball_init(const struct device *dev)
{
	const struct az1uball_config *config = dev->config;
	struct az1uball_data *data = dev->data;
	int ret;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	data->dev = dev;
	data->prev_button = 0;

	/* Initialize az1uball to normal mode */
	uint8_t mode_cmd = AZ1UBALL_CMD_NORMAL_MODE;
	ret = i2c_write_dt(&config->i2c, &mode_cmd, 1);
	if (ret < 0) {
		LOG_WRN("Failed to set az1uball mode: %d", ret);
		/* Continue anyway - module may work without explicit mode setting */
	}

	/* Initialize and schedule polling work */
	k_work_init_delayable(&data->work, az1uball_work_handler);
	k_work_schedule(&data->work, K_MSEC(CONFIG_AZ1UBALL_POLL_INTERVAL_MS));

	LOG_INF("az1uball initialized");
	return 0;
}

#define AZ1UBALL_INST(n)                                                    \
	static struct az1uball_data az1uball_data_##n;                      \
	static const struct az1uball_config az1uball_config_##n = {         \
		.i2c = I2C_DT_SPEC_INST_GET(n),                             \
		.invert_x = DT_INST_PROP(n, invert_x),                      \
		.invert_y = DT_INST_PROP(n, invert_y),                      \
		.swap_xy = DT_INST_PROP(n, swap_xy),                        \
	};                                                                  \
	DEVICE_DT_INST_DEFINE(n, az1uball_init, NULL,                       \
			      &az1uball_data_##n, &az1uball_config_##n,     \
			      POST_KERNEL, CONFIG_AZ1UBALL_THREAD_PRIORITY, \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(AZ1UBALL_INST)
