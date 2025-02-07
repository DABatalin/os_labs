set (CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.27)
project(kp)

find_package(cppzmq)

add_compile_options(-pthread)

add_executable(kp_client ./src/client.cpp)
add_executable(kp_server ./main.cpp)


target_link_libraries(kp_client PRIVATE cppzmq)
target_link_libraries(kp_server PRIVATE cppzmq)