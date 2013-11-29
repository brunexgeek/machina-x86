include_directories(${CMAKE_SOURCE_DIR}/src/bin/ar)

file(GLOB ar_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(ar ${ar_sources})
set_target_properties( ar PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GENERATED_DIR}/linux/bin")
