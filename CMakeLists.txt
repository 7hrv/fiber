cmake_minimum_required(VERSION 3.20)
project(Fiber
  VERSION 0.0.0
  LANGUAGES CXX ASM
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

add_library(fiber STATIC
  src/fiber.cpp
  src/scheduler.cpp
  src/context_switch.S
)

target_include_directories(fiber PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(fiber PUBLIC cxx_std_23)
add_library(Fiber::fiber ALIAS fiber)

# add_executable(fiber_demo examples/main.cpp)
# target_link_libraries(fiber_demo PRIVATE fiber)

option(FIBER_BUILD_BENCHMARKS "Build benchmarks" ON)
if (FIBER_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
