cmake_minimum_required(VERSION 3.8.0)

project(C_HTTP_server VERSION 0.2.0 LANGUAGES C)

# alter this variable to change example filename to compile
set(EXAMPLE_FILE basic)

string(CONCAT EXAMPLE_PATH example/ ${EXAMPLE_FILE} .c)
string(TOUPPER ${EXAMPLE_FILE} EXAMPLE_FILE)

add_compile_definitions(${EXAMPLE_FILE})
add_executable(C_HTTP_server ${EXAMPLE_PATH})

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
${SRC_DIR}/path.c
)