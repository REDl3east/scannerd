include(FetchContent)

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  uv
  GIT_REPOSITORY https://github.com/libuv/libuv
  GIT_TAG e9f29cb   # v1.48.0: 2024.02.07, Version 1.48.0 (Stable)
  SOURCE_DIR ${UV_SRC_DIR}
)

FetchContent_Declare(
  argtable3
  GIT_REPOSITORY https://github.com/argtable/argtable3
  GIT_TAG f25c624   # v3.2.2
  SOURCE_DIR ${ARGTABLE_SRC_DIR}
)

FetchContent_MakeAvailable(uv argtable3)