cmake_minimum_required(VERSION 3.16)
project(trinity_consumer LANGUAGES C)

find_package(Trinity CONFIG REQUIRED)

add_executable(trinity_consumer consumer.c)
target_link_libraries(trinity_consumer PRIVATE Trinity::trinity)