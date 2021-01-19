cmake_minimum_required(VERSION 3.17)
project(publisher_packager C)

set(CMAKE_C_STANDARD 99)

add_executable(publisher_packager main.c producerConsumer.c queue.c)