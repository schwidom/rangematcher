project(rangematcher)
cmake_minimum_required(VERSION 2.8)

set( CMAKE_VERBOSE_MAKEFILE 1 )

# set( CMAKE_CXX_COMPILER /usr/bin/clang++-3.5 )
# set( CMAKE_CXX_COMPILER /usr/bin/g++-4.9 )

add_compile_options( -Wall -std=c++14 )

add_compile_options( -I ../../src )

add_compile_options( -I/usr/include/lua5.2 )

add_compile_options( -ggdb -Og )

aux_source_directory( ../src SRC )
aux_source_directory( ../src/lua LUA )
aux_source_directory( ../src/main SRC_MAIN )
aux_source_directory( ../src/tests SRC_TESTS )

set( PROJECT_MAIN_NAME ${PROJECT_NAME}-main )
set( PROJECT_TESTS_NAME ${PROJECT_NAME}-tests )

add_executable( ${PROJECT_MAIN_NAME} ${SRC} ${LUA} ${SRC_MAIN})
target_link_libraries( ${PROJECT_MAIN_NAME} lua5.2 )

add_executable( ${PROJECT_TESTS_NAME} ${SRC} ${LUA} ${SRC_TESTS})
target_link_libraries( ${PROJECT_TESTS_NAME} lua5.2 )

