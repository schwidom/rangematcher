project(rangematcher)
cmake_minimum_required(VERSION 2.8)

add_compile_options( -Wall -std=c++14)
add_compile_options( -ggdb -O0)

aux_source_directory( .. SRC)

add_executable( ${PROJECT_NAME} ${SRC} )

