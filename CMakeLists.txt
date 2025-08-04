cmake_minimum_required(VERSION 3.10)
project(vector)
include_directories(include)
set(CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(vector main.cpp ${SRC_FILES})
