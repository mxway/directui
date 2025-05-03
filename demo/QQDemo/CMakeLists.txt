project(QQDemo)

link_libraries(DuiLib)

FILE(GLOB QQ_DEMO_SRC_FILES "*.cpp")

add_executable(QQDemo ${QQ_DEMO_SRC_FILES})
