cmake_minimum_required(VERSION 3.25)

project(bruce_lhme_external_consumer LANGUAGES CXX)

find_package(bruce_lhme 0.1 CONFIG REQUIRED)

add_executable(bruce_lhme_external_consumer main.cpp)
target_link_libraries(bruce_lhme_external_consumer PRIVATE bruce_lhme::core)
