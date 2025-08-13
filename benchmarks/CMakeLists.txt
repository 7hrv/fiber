find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS context)

add_executable( bench_handoff
  src/benchmarks_threads.cpp
  src/benchmarks_fibers.cpp
  src/benchmarks_boost.cpp
  src/benchmarks_coroutine.cpp
  src/main.cpp
)

target_include_directories(bench_handoff PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bench_handoff PRIVATE
  fiber
  Threads::Threads
  Boost::context
)

