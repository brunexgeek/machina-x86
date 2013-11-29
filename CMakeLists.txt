project(Machna)
cmake_minimum_required(VERSION 2.6)
set(GENERATED_DIR "${CMAKE_SOURCE_DIR}/generated")

add_subdirectory(${CMAKE_SOURCE_DIR}/src/bin/cc)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/bin/as)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/bin/ar)
