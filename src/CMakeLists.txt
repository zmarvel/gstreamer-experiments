add_executable(${PROJECT_NAME} main.cpp pipeline.cpp frame_source.cpp file_frame_source.cpp config.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GSTREAMERMM_LIBRARIES} BlockingCollection pthread)
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

