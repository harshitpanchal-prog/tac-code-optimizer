cmake_minimum_required(VERSION 3.15)
project(TACOptimizer CXX)

# ── C++ standard ────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source files ────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/parser.cpp
    src/basicblock.cpp
    src/cfg.cpp
    src/optimizer.cpp
    src/utils.cpp
)

# ── Executable ──────────────────────────────────────────────
add_executable(optimizer ${SOURCES})

# ── Include directory ───────────────────────────────────────
target_include_directories(optimizer PRIVATE include)

# ── Compiler warnings ───────────────────────────────────────
if (MSVC)
    target_compile_options(optimizer PRIVATE /W4)
else()
    target_compile_options(optimizer PRIVATE -Wall -Wextra)
endif()

# ── Place the binary in the project root (not in build/) ────
#    This ensures Node.js server.js finds it as ./optimizer
set_target_properties(optimizer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

message(STATUS "CMake configured. Run 'cmake --build .' to compile.")
