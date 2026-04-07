cmake_minimum_required(VERSION 3.16)
project(cpp_library LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bio_headers INTERFACE)
target_include_directories(bio_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bio_example examples/basic_usage.cpp)
target_link_libraries(bio_example PRIVATE bio_headers)
