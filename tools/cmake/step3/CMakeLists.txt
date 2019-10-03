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

enable_testing()

add_test(NAME Runs COMMAND Tutorial 25)

add_test(NAME Usage COMMAND Tutorial)
set_tests_properties(Usage PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*<number>")

function(do_test target arg result)
  add_test(NAME Sqrt${arg} COMMAND ${target} ${arg})
  set_tests_properties(Sqrt${arg} PROPERTIES PASS_REGURAL_EXPRESSION ${result})
endfunction(do_test)

do_test(Tutorial 4 "4 is 2")
do_test(Tutorial 9 "9 is 3")
do_test(Tutorial 5 "5 is 2.236")
do_test(Tutorial 7 "7 is 2.645")
do_test(Tutorial 25 "25 is 5")
do_test(Tutorial -25 "-25 is [-nan|nan|0]")
do_test(Tutorial 0.0001 "0.0001 is 0.01")