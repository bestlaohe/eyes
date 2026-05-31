cmake_minimum_required(VERSION 3.24)

idf_component_register(
    SRCS "main.cpp" "eye_functions.cpp" "user.cpp" "config.cpp"
    INCLUDE_DIRS "." "data"
)