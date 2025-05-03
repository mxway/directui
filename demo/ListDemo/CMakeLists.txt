project(ListDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")
link_libraries(DuiLib)
add_executable(ListDemo ${LOCAL_SRC_FILES})

add_custom_command(TARGET ListDemo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${PROJECT_SOURCE_DIR}/ListRes ${CMAKE_BINARY_DIR}/ListRes)
