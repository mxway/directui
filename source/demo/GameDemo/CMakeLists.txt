project(GameDemo)

FILE(GLOB GAME_SRC_FILES "*.cpp")
link_libraries(DuiLib)

add_executable(GameDemo ${GAME_SRC_FILES})

add_custom_command(TARGET GameDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/GameRes ${CMAKE_BINARY_DIR}/GameRes)
