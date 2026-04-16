project(RichEditDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")
link_libraries(DuiLib)
add_executable(RichEditDemo ${LOCAL_SRC_FILES})

add_custom_command(TARGET RichEditDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/RichEditRes ${CMAKE_BINARY_DIR}/RichEditRes)

add_custom_command(TARGET RichEditDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/red_chamber.txt ${CMAKE_BINARY_DIR}/red_chamber.txt)