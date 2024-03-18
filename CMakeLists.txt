cmake_minimum_required(VERSION 3.0.0)
project(scannerd VERSION 0.1.0 LANGUAGES C)

set(UV_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/libuv)
set(ARGTABLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/argtable3)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/evdev.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/run.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/query.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sv.c
)

add_executable(scannerd ${MAIN_SRC})
target_link_libraries(scannerd PRIVATE uv)
target_link_libraries(scannerd PRIVATE argtable3)
target_include_directories(scannerd PRIVATE ${ARGTABLE_SRC_DIR}/src)
