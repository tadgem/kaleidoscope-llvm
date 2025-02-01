cmake_minimum_required(VERSION 3.20)
project(kaleidoscope-llvm)

find_package(LLVM REQUIRED)

file(GLOB_RECURSE KAL_SRC_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" src/*.cpp)
file(GLOB_RECURSE KAL_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" include /*.h)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(kaleidoscope-compiler ${KAL_SRC_FILES} ${KAL_HEADER_FILES})
target_include_directories(kaleidoscope-compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)