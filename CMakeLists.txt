cmake_minimum_required(VERSION 3.11)
project(directui)

set(CMAKE_CXX_STANDARD 11)

set(GTK_BACKEND ON)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    FILE(GLOB OS_SRC_FILES "os/win32/*.cpp")
    link_libraries(comctl32)
    link_libraries(shlwapi)
    link_libraries(imm32)
else()
    FILE(GLOB OS_SRC_FILES "os/linux/*.cpp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie")
    find_package(PkgConfig REQUIRED)
    if(GTK_BACKEND)
        add_definitions(-DGTK_BACKEND)
        # linux操作系统平台下生成的GUI进程，能够通过双击打开进程。
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        include_directories(${GTK3_INCLUDE_DIRS})
        link_directories(${GTK3_LIBRARY_DIRS})
        link_libraries(${GTK3_LIBRARIES})
    else()
        pkg_check_modules(PANGO REQUIRED pango)
        include_directories(${PANGO_INCLUDE_DIRS})
        link_libraries(${PANGO_LIBRARIES})
    endif()
endif()

FILE(GLOB_RECURSE SRC_FILES "src/*.cpp")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDebug)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -fprofile-arcs -ftest-coverage")
    endif()
else()
    ## 对于release版本，windows下不显示控制台窗口。
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
        else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
		endif()
    endif()
endif()

include_directories(third-party/zlib-1.3.1)
include_directories(third-party/zlib-1.3.1/minizip)
add_subdirectory(third-party/zlib-1.3.1)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # msvc不支持动态库生成，目前仅支持静态库的生成
    add_library(DuiLib ${OS_SRC_FILES} ${SRC_FILES})
else()
    add_library(DuiLib SHARED ${OS_SRC_FILES} ${SRC_FILES})
endif()

target_link_libraries(DuiLib PRIVATE zlibstatic)

include_directories(include)
add_subdirectory(demo)
