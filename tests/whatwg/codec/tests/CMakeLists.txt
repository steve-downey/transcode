# 1. Find the Python3 Interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 2. Define our input paths
set(WPT_INDEXES_JSON "${CMAKE_SOURCE_DIR}/wpt/encoding/indexes.json")
set(PYTHON_GENERATOR "${CMAKE_SOURCE_DIR}/tools/generate_indices.py")

# 3. Define the output directory (inside the build tree, keeping source clean)
set(GENERATED_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# 4. Explicitly list the expected outputs.
# Explicit outputs are crucial so Ninja/Make know exactly what this command produces.
set(GENERATED_BIN_FILES
    "${GENERATED_BIN_DIR}/big5.bin"
    "${GENERATED_BIN_DIR}/shift_jis.bin"
    # Add other index files here as you parse them (gb18030.bin, euc_kr.bin, etc.)
)

# 5. The custom command that runs the Python generator
add_custom_command(
    OUTPUT ${GENERATED_BIN_FILES}
    COMMAND Python3::Interpreter "${PYTHON_GENERATOR}" "${WPT_INDEXES_JSON}" "${GENERATED_BIN_DIR}"
    DEPENDS "${WPT_INDEXES_JSON}" "${PYTHON_GENERATOR}"
    COMMENT "Generating C++26 #embed binary indices from WPT JSON..."
    VERBATIM
)

# 6. Create a custom target that represents the completion of the data generation
add_custom_target(wpt_indices_data DEPENDS ${GENERATED_BIN_FILES})

# ---------------------------------------------------------
# Test Executable Configuration
# ---------------------------------------------------------

# Replace with your actual test source files
add_executable(whatwg_codec_tests
    test_main.cpp
    # ... other test sources
)

# Enforce C++26 for #embed and consteval
target_compile_features(whatwg_codec_tests PRIVATE cxx_std_26)

# CRITICAL: Tell CMake that the executable CANNOT compile until the binary data exists
add_dependencies(whatwg_codec_tests wpt_indices_data)

# Include directories so #embed can find the files.
# Because we include CMAKE_CURRENT_BINARY_DIR, your C++ code can just do:
# #embed "generated/big5.bin"
target_include_directories(whatwg_codec_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
