cmake_minimum_required(VERSION 3.5)

project("http_server")

set(HTTP_SERVER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(HTTP_SERVER_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(${HTTP_SERVER_HEADER_DIR})

add_executable(server ${CMAKE_SOURCE_DIR}/main.c ${HTTP_SERVER_SOURCE_DIR}/http.c)