cmake_minimum_required(VERSION 3.24)

idf_component_register(
    SRCS "main.cpp" "eye_functions.cpp" "user.cpp" "config.cpp" "display_port.cpp"
    INCLUDE_DIRS "." "data"
    REQUIRES arduino esp_lcd esp_lcd_gc9d01 driver
)
