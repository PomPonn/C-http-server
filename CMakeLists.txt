cmake_minimum_required(VERSION 3.8.0)

project(C_HTTP_server VERSION 0.2.0 LANGUAGES C)

add_executable(C_HTTP_server example/main.c)

set(SRC_DIR ${CMAKE_HOME_DIRECTORY}/src)

target_include_directories(C_HTTP_server PUBLIC inc/)

target_sources(C_HTTP_server PUBLIC
${SRC_DIR}/base_server.c
${SRC_DIR}/http_server.c
${SRC_DIR}/utils.c
${SRC_DIR}/httpprot.c
${SRC_DIR}/url.c
${SRC_DIR}/error.c
${SRC_DIR}/pair.c
)