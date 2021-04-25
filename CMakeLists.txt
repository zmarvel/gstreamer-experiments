cmake_minimum_required(VERSION 3.13)
project(camcoder CXX)

option(BUILD_TUTORIALS "Build programs in gstreamer-tutorials/" ON)

include(FindPkgConfig)

pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)

if (BUILD_TUTORIALS)
  add_subdirectory(gstreamer-tutorials)
endif (BUILD_TUTORIALS)

add_subdirectory(lib/sockpp)
add_subdirectory(lib/toml11)

add_library(BlockingCollection INTERFACE)
target_include_directories(BlockingCollection INTERFACE lib/BlockingCollection)

add_compile_options(-Wall -Wextra)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE sockpp toml11)
