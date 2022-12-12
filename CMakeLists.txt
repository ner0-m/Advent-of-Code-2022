cmake_minimum_required(VERSION 3.18)
project(AOC2022)

# Set to C++20 and enforce it
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# create the `compile_commands.json` for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# nice colored output :-)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
  set(CMAKE_COLOR_DIAGNOSTICS ON)
endif()

function(add_day day)
  set(exec day${day})
  add_executable(${exec} src/day${day}.cpp ${ARGN})
  target_link_libraries(${exec} common)
  target_include_directories(
    ${exec}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/range-v3/include>
           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/fmt/include>
           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
endfunction()

add_library(common src/common/input.cpp)
target_include_directories(
  common PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

add_day("02")
add_day("03")
add_day("04")
add_day("05")
add_day("06")
add_day("07")
add_day("08")
add_day("09")
add_day("10")
add_day("11")
add_day("12")
