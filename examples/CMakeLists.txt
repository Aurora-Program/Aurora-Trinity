cmake_minimum_required(VERSION 3.16)
project(TrinityConsumer LANGUAGES C)

find_package(Trinity 0.1 REQUIRED CONFIG)

add_executable(trinity_smoke trinity_smoke.c)
target_link_libraries(trinity_smoke PRIVATE Trinity::trinity)

add_executable(genesis_smoke genesis_smoke.c)
target_link_libraries(genesis_smoke PRIVATE Trinity::genesis)
