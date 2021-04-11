cmake_minimum_required(VERSION 3.13)
project(basic04 CXX)

include(FindPkgConfig)

pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
  CMAKE_CXX_STANDARD 17
  CMAKE_CXX_STANDARD_REQUIRED ON
)
