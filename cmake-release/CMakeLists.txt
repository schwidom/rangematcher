project(rangematcher)
cmake_minimum_required(VERSION 2.8)

add_compile_options( -Wall -std=c++14)
add_compile_options( -O3)

aux_source_directory( ../src SRC)

add_executable( ${PROJECT_NAME} ${SRC} )

