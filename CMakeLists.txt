cmake_minimum_required(VERSION 3.8.0)

project(C_HTTP_server VERSION 0.1.0 LANGUAGES C)

add_executable(C_HTTP_server src/main.c)

set(SRC_DIR ${CMAKE_HOME_DIRECTORY}/src)

target_sources(C_HTTP_server PUBLIC
${SRC_DIR}/server.c
)