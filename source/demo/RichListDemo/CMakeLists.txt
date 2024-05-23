project(RichListDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")
link_libraries(DuiLib)
add_executable(RichListDemo ${LOCAL_SRC_FILES})

add_custom_command(TARGET RichListDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/RichListRes ${CMAKE_BINARY_DIR}/RichListRes)