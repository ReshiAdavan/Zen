cmake_minimum_required(VERSION 2.8)

project(zen)
set (VERSION 0.0.6)

OPTION(DEBUG "add debug flags" OFF)

add_definitions("-D_DEFAULT_SOURCE")
find_package(OpenSSL REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(include)
file(GLOB SOURCES src/*.c)

# Executable
add_executable(zen ${SOURCES} include/zen_internal.h)

if (DEBUG)
    message(STATUS "Configuring build for debug")
    TARGET_LINK_LIBRARIES(zen pthread ssl crypto)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused -Werror -pedantic \
    -Wno-unused-result -std=c11 -ggdb -fsanitize=address \
    -fsanitize=undefined -fno-omit-frame-pointer -pg")
else (DEBUG)
    message(STATUS "Configuring build for production")
    TARGET_LINK_LIBRARIES(zen pthread ssl crypto)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused -Werror -pedantic \
    -Wno-unused-result -std=c11 -O3")
endif (DEBUG)
