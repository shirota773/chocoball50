# Copyright (c) 2025 The ZMK Contributors
# SPDX-License-Identifier: MIT

# Add custom drivers subdirectory
add_subdirectory(drivers)

if(CONFIG_AZ1UBALL)
  message(STATUS "az1uball driver enabled - compiling az1uball.c")
  zephyr_library_sources(az1uball.c)
endif()
