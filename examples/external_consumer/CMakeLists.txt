cmake_minimum_required(VERSION 3.25)

project(axiom_lhme_external_consumer LANGUAGES CXX)

find_package(axiom_lhme 0.1 CONFIG REQUIRED)

add_executable(axiom_lhme_external_consumer main.cpp)
target_link_libraries(axiom_lhme_external_consumer PRIVATE axiom_lhme::core)
