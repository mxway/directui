project(360SafeDemo)

FILE(GLOB LOCAL_SRC_FILES "*.cpp")

add_executable(360SafeDemo ${LOCAL_SRC_FILES} ${SRC_FILES} ${OS_SRC_FILES})