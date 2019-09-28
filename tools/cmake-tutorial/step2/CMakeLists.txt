cmake_minimum_required(VERSION 2.6)
project(Tutorial)

set(Tutorial_VERSION_MAJOR 1)
set(Tutorial_VERSION_MINOR 0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(
    ${PROJECT_SOURCE_DIR}/TutorialConfig.h.in
    ${PROJECT_BINARY_DIR}/TutorialConfig.h
)

if(USE_MYMATH)
  add_subdirectory(math_functions)
  list(APPEND EXTRA_LIBS math_functions)
  list(APPEND EXTRA_INCLUDES ${PROJECT_SOURCE_DIR}/math_functions)
endif()

add_executable(Tutorial tutorial.cc)
target_link_libraries(Tutorial ${EXTRA_LIBS})
target_include_directories(Tutorial PUBLIC
                           ${PROJECT_BINARY_DIR}
                           ${EXTRA_INCLUDES})