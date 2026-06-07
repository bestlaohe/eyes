cmake_minimum_required(VERSION 3.24)

idf_component_register(
    SRCS "main.cpp" "gif_player.cpp" "gif_frames.cpp" "config.cpp" "display_port.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino esp_lcd esp_lcd_gc9d01 driver
)

set(GIF_FRAMES_H "${CMAKE_CURRENT_SOURCE_DIR}/data/gif_frames.h")
set(GIF_FRAMES_INC "${CMAKE_CURRENT_SOURCE_DIR}/data/gif_frames.inc")
set(GIF_GEN_SCRIPT "${CMAKE_SOURCE_DIR}/tools/gif_to_frames.py")
set(GIF_MANIFEST "${CMAKE_SOURCE_DIR}/assets/salary_cat/manifest.json")
set(GIF_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

file(GLOB GIF_VOL1_ASSETS "${CMAKE_SOURCE_DIR}/assets/salary_cat/vol1/*.gif")
file(GLOB GIF_VOL2_ASSETS "${CMAKE_SOURCE_DIR}/assets/salary_cat/vol2/*.gif")
file(GLOB GIF_VOL3_ASSETS "${CMAKE_SOURCE_DIR}/assets/salary_cat/vol3/*.gif")

if(NOT PYTHON)
  if(DEFINED ENV{PYTHON})
    set(PYTHON "$ENV{PYTHON}")
  else()
    find_program(PYTHON python3 python)
  endif()
endif()

add_custom_command(
  OUTPUT ${GIF_FRAMES_H} ${GIF_FRAMES_INC}
  COMMAND ${PYTHON} ${GIF_GEN_SCRIPT}
  DEPENDS
    ${GIF_CONFIG}
    ${GIF_GEN_SCRIPT}
    ${GIF_MANIFEST}
    ${GIF_VOL1_ASSETS}
    ${GIF_VOL2_ASSETS}
    ${GIF_VOL3_ASSETS}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Generate gif_frames from config.h GIF_CLIP_VOL/INDEX"
  VERBATIM
)

add_custom_target(gif_frames_gen DEPENDS ${GIF_FRAMES_H} ${GIF_FRAMES_INC})
add_dependencies(${COMPONENT_LIB} gif_frames_gen)

set_source_files_properties(gif_frames.cpp PROPERTIES
  OBJECT_DEPENDS "${GIF_FRAMES_H};${GIF_FRAMES_INC}")
