cmake_minimum_required(VERSION 2.8.12)
project(mapper)

add_definitions("-std=c++20")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(mapper main.cpp)
target_link_libraries(mapper CONAN_PKG::rapidxml)
target_link_libraries(mapper CONAN_PKG::libarchive)
target_link_libraries(mapper CONAN_PKG::fmt)

target_compile_options(mapper PRIVATE -Wall -Wextra -Wpedantic -Werror)
