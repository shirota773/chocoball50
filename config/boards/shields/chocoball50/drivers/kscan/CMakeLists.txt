# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_KSCAN_GPIO_DUPLEX)
  zephyr_library()
  zephyr_library_sources(kscan_gpio_duplex.c)
endif()
