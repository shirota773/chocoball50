# Copyright (c) 2025 The ZMK Contributors
# SPDX-License-Identifier: MIT

# Custom kscan driver for Japanese duplex matrix
zephyr_library_sources(drivers/kscan/kscan_gpio_duplex.c)

# az1uball trackball driver
if(CONFIG_AZ1UBALL)
  message(STATUS "az1uball driver enabled - compiling az1uball.c")
  zephyr_library_sources(az1uball.c)
endif()
