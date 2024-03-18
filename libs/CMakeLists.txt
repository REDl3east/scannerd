include(FetchContent)

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  uv
  GIT_REPOSITORY https://github.com/libuv/libuv
  GIT_TAG e9f29cb # v1.48.0: 2024.02.07, Version 1.48.0 (Stable)
  SOURCE_DIR ${UV_SRC_DIR}
)

set(BUILD_SHARED_LIBS         OFF CACHE BOOL "" FORCE)
set(ARGTABLE3_ENABLE_CONAN    OFF CACHE BOOL "" FORCE)
set(ARGTABLE3_ENABLE_TESTS    OFF CACHE BOOL "" FORCE)
set(ARGTABLE3_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  argtable3
  GIT_REPOSITORY https://github.com/argtable/argtable3
  GIT_TAG f25c624 # v3.2.2
  SOURCE_DIR ${ARGTABLE_SRC_DIR}
)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio/
  GIT_TAG  ed5db1b # main 03/18/24
  SOURCE_DIR ${ASIO_SRC_DIR}
)

FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow
  GIT_TAG  05f7f33 # v1.1.0
  SOURCE_DIR ${CROW_SRC_DIR}
)

FetchContent_MakeAvailable(uv argtable3 asio crow)