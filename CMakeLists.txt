cmake_minimum_required(VERSION 3.16)
project(cpplibs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(cpplibs INTERFACE)
target_include_directories(cpplibs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bio_example examples/bio_example.cpp)
target_link_libraries(bio_example PRIVATE cpplibs)

add_executable(obj2str_example examples/obj2str_example.cpp)
target_link_libraries(obj2str_example PRIVATE cpplibs)

add_executable(util_example examples/util_example.cpp)
target_link_libraries(util_example PRIVATE cpplibs)
