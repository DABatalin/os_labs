cmake_minimum_required(VERSION 3.27)
project(lab1 VERSION 0.1.0 LANGUAGES C CXX)

add_compile_options(-pthread)
add_executable(child1 child1.c)
add_executable(child2 child2.c)
add_executable(lab3 main.c)