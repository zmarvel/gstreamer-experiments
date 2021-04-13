cmake_minimum_required(VERSION 3.13)
project(camcoder CXX)

option(BUILD_TUTORIALS "Build programs in gstreamer-tutorials/" ON)

include(FindPkgConfig)

pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)

if (BUILD_TUTORIALS)
  add_subdirectory(gstreamer-tutorials)
endif (BUILD_TUTORIALS)

add_subdirectory(lib/sockpp)

add_subdirectory(src)
