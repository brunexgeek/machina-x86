include_directories(${CMAKE_SOURCE_DIR}/src/bin/as)

file(GLOB as_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/output/*.c")

add_definitions(-DOF_ONLY)
add_definitions(-DOF_ELF32)
add_definitions(-DOF_WIN32)
add_definitions(-DOF_COFF)
add_definitions(-DOF_OBJ)
add_definitions(-DOF_BIN)
add_definitions(-DOF_DBG)
add_definitions(-DOF_DEFAULT=of_elf32)
add_definitions(-DHAVE_SNPRINTF)
add_definitions(-DHAVE_VSNPRINTF)
add_definitions(-DUSE_LOCAL_HEAP)

add_executable(as ${as_sources})
set_target_properties( as PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GENERATED_DIR}/linux/bin")
