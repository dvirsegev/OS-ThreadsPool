cmake_minimum_required(VERSION 3.14)
project(ex4)

SET(CMAKE_C_FLAGS -pthread)

find_package (Threads)
add_executable(ex4 threadPool.c threadPool.h osqueue.c osqueue.h file1.c)