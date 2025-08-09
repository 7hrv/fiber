cmake_minimum_required(VERSIION 3.15)
project(Fiber VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable()

target_include_directories(fiber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
