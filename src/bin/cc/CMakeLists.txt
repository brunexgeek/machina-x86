include_directories(${CMAKE_SOURCE_DIR}/src/bin/cc)

file(GLOB cc_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(cc ${cc_sources})
set_target_properties( cc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GENERATED_DIR}/linux/bin")
