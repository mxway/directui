project(360SafeDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")

link_libraries(DuiLib)

add_executable(360SafeDemo ${LOCAL_SRC_FILES})

add_custom_command(TARGET 360SafeDemo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${PROJECT_SOURCE_DIR}/360SafeRes ${CMAKE_BINARY_DIR}/360SafeRes)
