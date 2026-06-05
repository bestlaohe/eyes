# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# 使用本地 components（mdns、esp_lcd_gc9d01），避免在线组件下载校验失败
set(ENV{IDF_COMPONENT_MANAGER} "0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(eyes)


