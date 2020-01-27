cmake_minimum_required(VERSION 3.5)

project("http_server")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
add_definitions(-D_GNU_SOURCE)

add_executable(server ${CMAKE_SOURCE_DIR}/main.c ${CMAKE_SOURCE_DIR}/http.c)