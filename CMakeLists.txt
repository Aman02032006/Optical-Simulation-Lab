cmake_minimum_required(VERSION 3.15)
project(QISS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# 1. Collect your project sources
# -----------------------------
file(GLOB_RECURSE QISS_SOURCES src/*.cpp)

add_executable(QISS ${QISS_SOURCES})

# -----------------------------
# 2. Include directories
# -----------------------------
# Your headers
target_include_directories(QISS PRIVATE include)

# FFTW headers
set(FFTW_ROOT "${PROJECT_SOURCE_DIR}/extern/FFTW")
set(FFTW_INCLUDE_DIR "${FFTW_ROOT}/include")
target_include_directories(QISS PRIVATE ${FFTW_INCLUDE_DIR})

# ImGui + ImPlot headers
target_include_directories(QISS PRIVATE extern/ImGui extern/ImPlot)

# Include directories for the ImGui backend headers
target_include_directories(QISS PRIVATE
    ${CMAKE_SOURCE_DIR}/extern/ImGui/backends
)

# -----------------------------
# 3. Add external sources manually
# -----------------------------
# ImGui sources
file(GLOB IMGUI_SOURCES
    extern/ImGui/*.cpp
)

# ImPlot sources
file(GLOB IMPLOT_SOURCES
    extern/ImPlot/*.cpp
)

# Add ImGui backends (only the .cpp files)
set(IMGUI_BACKENDS
    ${CMAKE_SOURCE_DIR}/extern/ImGui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/extern/ImGui/backends/imgui_impl_opengl3.cpp
)

# Add all external sources to your executable
target_sources(QISS PRIVATE 
    ${IMGUI_SOURCES} 
    ${IMPLOT_SOURCES}
    ${IMGUI_BACKENDS}
)

# -----------------------------
# 4. Add External Libraries (Subdirectories)
# -----------------------------

# --- GLFW ---
set(GLFW_ROOT "${PROJECT_SOURCE_DIR}/extern/GLFW")
add_subdirectory(${GLFW_ROOT})

# --- OpenGL ---
find_package(OpenGL REQUIRED)

# --- Matplotplusplus ---
# Disable its examples and tests to speed up your build
set(MATPLOTPP_BUILD_EXAMPLES OFF CACHE BOOL "Build Matplot++ examples" FORCE)
set(MATPLOTPP_BUILD_TESTS OFF CACHE BOOL "Build Matplot++ tests" FORCE)
add_subdirectory(extern/Matplotplusplus)

# --- FFTW ---
set(FFTW_LIB_DIR "${FFTW_ROOT}/lib")
set(FFTW_LIB "${FFTW_LIB_DIR}/fftw3.lib")

if(NOT EXISTS ${FFTW_LIB}) 
    message(FATAL_ERROR "FFTW import library not found: ${FFTW_LIB}. You must run lib.exe on the .def file.")
endif()
link_directories(${FFTW_LIB_DIR}) # Lets target_link_libraries find fftw3

# -----------------------------
# 5. Link All Libraries
# -----------------------------
target_link_libraries(QISS PRIVATE 
    # Matplot++ (handles its own dependencies)
    matplot

    # GLFW (for windowing)
    glfw

    # OpenGL (for rendering)
    OpenGL::GL

    # FFTW (for physics)
    fftw3
)

# -----------------------------
# 6. Optional compile definitions
# -----------------------------
target_compile_definitions(QISS PRIVATE _CRT_SECURE_NO_WARNINGS)

# -----------------------------
# 7. Post-Build Commands
# -----------------------------
# Copy runtime DLLs
add_custom_command(TARGET QISS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FFTW_LIB_DIR}/libfftw3-3.dll
        $<TARGET_FILE_DIR:QISS>
)