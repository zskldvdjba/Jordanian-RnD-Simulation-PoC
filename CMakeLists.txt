cmake_minimum_required(VERSION 3.20)

project(Jordanian_RnD_Simulation_PoC
    VERSION 0.1.0
    DESCRIPTION "Jordanian Advanced Industrial R&D Simulation & Digital Engineering PoC"
    LANGUAGES CXX
)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common compilation options
if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Testing option
option(ENABLE_SIM_TESTING "Build simulation unit and integration tests" ON)

# Subdirectories will be enabled as implementation proceeds
# add_subdirectory(src)
# if(ENABLE_SIM_TESTING)
#     enable_testing()
#     add_subdirectory(tests)
# endif()
