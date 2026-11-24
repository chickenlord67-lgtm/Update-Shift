cmake_minimum_required(VERSION 3.21)
project(update-shift VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(geode-sdk)
setup_geode_mod(${PROJECT_NAME})
