cmake_minimum_required(VERSION 3.24)

idf_component_register(
    SRCS "main.cpp" "heart.cpp" "config.cpp" "display_port.cpp"
    INCLUDE_DIRS "." 
    REQUIRES arduino esp_lcd esp_lcd_gc9d01 driver
)
