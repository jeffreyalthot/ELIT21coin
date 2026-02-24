cmake_minimum_required(VERSION 3.16)
project(ELIT21coin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ELIT21_BUILD_TESTS "Build ELIT21coin tests" ON)

add_library(elit21core
    src/block.cpp
    src/codec.cpp
    src/blockchain.cpp
)

target_include_directories(elit21core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(elit21core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(elit21_demo src/main.cpp)
target_link_libraries(elit21_demo PRIVATE elit21core)
target_compile_options(elit21_demo PRIVATE -Wall -Wextra -Wpedantic)

if(ELIT21_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(elit21_tests tests/test_elit21.cpp)
    target_link_libraries(elit21_tests PRIVATE elit21core)
    target_compile_options(elit21_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME elit21_tests COMMAND elit21_tests)
endif()
