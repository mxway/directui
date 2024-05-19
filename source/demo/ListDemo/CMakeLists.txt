project(ListDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")

add_executable(ListDemo ${LOCAL_SRC_FILES} ${SRC_FILES} ${OS_SRC_FILES})
