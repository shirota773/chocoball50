# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZMK_KSCAN_GPIO_DUPLEX kscan_gpio_duplex.c)
