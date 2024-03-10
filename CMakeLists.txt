cmake_minimum_required(VERSION 3.8.0)

project(C_HTTP_server VERSION 0.1.0 LANGUAGES C)

add_executable(C_HTTP_server src/main.c)
#add_executable(CLIENT src/client.c)

set(SRC_DIR ${CMAKE_HOME_DIRECTORY}/src)

target_include_directories(C_HTTP_server PUBLIC inc/)

target_sources(C_HTTP_server PUBLIC
${SRC_DIR}/server.c
${SRC_DIR}/utils.c
${SRC_DIR}/httpprot.c
)