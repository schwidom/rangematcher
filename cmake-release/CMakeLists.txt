project(rangematcher)
cmake_minimum_required(VERSION 2.8)

add_compile_options( -Wall -std=c++14 )

add_compile_options( -I /home/ox/dev-git/rangematcher-like-sgrep/src )
add_compile_options( -I/usr/include/lua5.2 )

add_compile_options( -O3 )

aux_source_directory( ../src SRC )
aux_source_directory( ../src/main SRC_MAIN )
aux_source_directory( ../src/tests SRC_TESTS )

set( PROJECT_MAIN_NAME ${PROJECT_NAME}-main )
set( PROJECT_TESTS_NAME ${PROJECT_NAME}-tests )

add_executable( ${PROJECT_MAIN_NAME} ${SRC} ${SRC_MAIN})
target_link_libraries( ${PROJECT_MAIN_NAME} lua5.2 )

add_executable( ${PROJECT_TESTS_NAME} ${SRC} ${SRC_TESTS})
target_link_libraries( ${PROJECT_TESTS_NAME} lua5.2 )

