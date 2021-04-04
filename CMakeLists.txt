cmake_minimum_required(VERSION 3.13)
project(video_encoding CXX)

option(BUILD_TUTORIALS "Build programs in gstreamer-tutorials/" ON)

include(FindPkgConfig)

pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)

if (BUILD_TUTORIALS)
  add_subdirectory(gstreamer-tutorials)
endif (BUILD_TUTORIALS)

