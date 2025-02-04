cmake_minimum_required(VERSION 3.20)
project(kaleidoscope-llvm)

find_package(LLVM REQUIRED CONFIG)

file(GLOB_RECURSE KAL_SRC_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" src/*.cpp)
file(GLOB_RECURSE KAL_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" include /*.h)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_definitions("/wd4624")
endif(WIN32)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(kaleidoscope-compiler ${KAL_SRC_FILES} ${KAL_HEADER_FILES})
target_include_directories(kaleidoscope-compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LLVM_INCLUDE_DIRS})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
message(STATUS "LLVM Definitions : ${LLVM_DEFINITIONS}")
target_compile_definitions(kaleidoscope-compiler PRIVATE ${LLVM_DEFINITIONS_LIST} /wd4624)

llvm_map_components_to_libnames(llvm_libs support core irreader orcjit native)
message(STATUS "LLVM Libs : ${llvm_libs}")
target_link_libraries(kaleidoscope-compiler ${llvm_libs})