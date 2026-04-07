cmake_minimum_required(VERSION 3.16)
project(cpp_library LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bio_headers INTERFACE)
target_include_directories(bio_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bio_example examples/basic_usage.cpp)
target_link_libraries(bio_example PRIVATE bio_headers)

add_executable(bio_example_variant examples/variant_usage.cpp)
target_link_libraries(bio_example_variant PRIVATE bio_headers)

add_executable(bio_test_basic tests/basic_roundtrip.cpp)
target_link_libraries(bio_test_basic PRIVATE bio_headers)
