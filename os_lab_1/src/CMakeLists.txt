cmake_minimum_required(VERSION 3.27)
project(lab1 VERSION 0.1.0 LANGUAGES C CXX)

# find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS})

add_executable(lab1 main.c)

# enable_testing()

# add_executable(tests01 ../test/tests01.c lower_spaces.c)
# target_link_libraries(tests01 ${GTEST_LIBRARIES})

# add_test(NAME BASIC COMMAND tests01)