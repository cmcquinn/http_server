cmake_minimum_required(VERSION 3.5)

project("http_server")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
add_definitions(-D_GNU_SOURCE)

set(MODULE_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(MODULE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MODULE_SRCS ${MODULE_SRC_DIR}/server.c ${MODULE_SRC_DIR}/http.c)

include_directories(${MODULE_INCLUDE_DIR})

add_executable(server ${CMAKE_SOURCE_DIR}/main.c ${MODULE_SRCS})