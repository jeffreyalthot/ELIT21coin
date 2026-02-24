cmake_minimum_required(VERSION 3.16)
project(ELIT21coin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ELIT21_BUILD_TESTS "Build ELIT21coin tests" ON)
option(ELIT21_ENABLE_SANITIZERS "Enable Address/Undefined sanitizers on supported compilers" OFF)
option(ELIT21_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(ELIT21_ENABLE_IPO "Enable interprocedural optimization (LTO) for release builds" OFF)
option(ELIT21_ENABLE_CLANG_TIDY "Run clang-tidy during compilation when available" OFF)

add_library(elit21_warnings INTERFACE)
if(MSVC)
    target_compile_options(elit21_warnings INTERFACE /W4 /permissive-)
    if(ELIT21_WARNINGS_AS_ERRORS)
        target_compile_options(elit21_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(elit21_warnings INTERFACE -Wall -Wextra -Wpedantic -Wconversion)
    if(ELIT21_WARNINGS_AS_ERRORS)
        target_compile_options(elit21_warnings INTERFACE -Werror)
    endif()
endif()

add_library(elit21core
    src/block.cpp
    src/codec.cpp
    src/blockchain.cpp
)

target_include_directories(elit21core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(elit21core PUBLIC elit21_warnings)

if(ELIT21_ENABLE_CLANG_TIDY)
    find_program(ELIT21_CLANG_TIDY_EXE NAMES clang-tidy)
    if(ELIT21_CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${ELIT21_CLANG_TIDY_EXE}")
    else()
        message(WARNING "ELIT21_ENABLE_CLANG_TIDY requested but clang-tidy not found")
    endif()
endif()

if(ELIT21_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(elit21core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(elit21core PRIVATE -fsanitize=address,undefined)
endif()

add_executable(elit21_demo src/main.cpp)
target_link_libraries(elit21_demo PRIVATE elit21core elit21_warnings)

if(ELIT21_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ELIT21_IPO_SUPPORTED OUTPUT ELIT21_IPO_ERROR)
    if(ELIT21_IPO_SUPPORTED)
        set_property(TARGET elit21core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET elit21_demo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "ELIT21_ENABLE_IPO requested but unsupported: ${ELIT21_IPO_ERROR}")
    endif()
endif()

if(ELIT21_BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_executable(elit21_tests tests/test_elit21.cpp)
    target_link_libraries(elit21_tests PRIVATE elit21core elit21_warnings)
    if(ELIT21_ENABLE_IPO AND ELIT21_IPO_SUPPORTED)
        set_property(TARGET elit21_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    add_test(NAME elit21_tests COMMAND elit21_tests)
endif()
