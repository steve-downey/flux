
add_library(nanobench nanobench.cpp)
target_include_directories(nanobench PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(internal-iteration-benchmark internal_iteration_benchmark.cpp)
target_link_libraries(internal-iteration-benchmark PUBLIC nanobench flux)

add_executable(sort-benchmark sort_benchmark.cpp)
target_link_libraries(sort-benchmark PUBLIC nanobench flux)

add_executable(multidimensional-memset-benchmark multidimensional_memset_benchmark.cpp multidimensional_memset_benchmark_kernels.cpp)
target_link_libraries(multidimensional-memset-benchmark PUBLIC nanobench flux)
