cmake_minimum_required(VERSION 3.18)
project(AOC2022)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

function(add_day day files)
  set(exec day${day})
  add_executable(${exec} ${files})
  target_include_directories(
    ${exec}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/range-v3/include>
           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/fmt/include>
           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
endfunction()

add_day("02" src/day02/main.cpp)
add_day("03" src/day03/main.cpp)
