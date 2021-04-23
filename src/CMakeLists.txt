add_executable(${PROJECT_NAME} main.cpp pipeline.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_LIBRARIES} BlockingCollection pthread)
set_target_properties(${PROJECT_NAME} PROPERTIES
  CMAKE_CXX_STANDARD 17
  CMAKE_CXX_STANDARD_REQUIRED ON
)

