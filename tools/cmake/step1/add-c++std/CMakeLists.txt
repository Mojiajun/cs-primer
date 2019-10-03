cmake_minimum_required(VERSION 2.6)
project(Tutorial)
set(Tutorial_VERSION_MAJOR 1)
set(Tutorial_VERSION_MINOR 0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TURE)

configure_file(
    ${PROJECT_SOURCE_DIR}/TutorialConfig.h.in
    ${PROJECT_BINARY_DIR}/TutorialConfig.h
)

include_directories(${PROJECT_BINARY_DIR})
add_executable(Tutorial tutorial.cc)