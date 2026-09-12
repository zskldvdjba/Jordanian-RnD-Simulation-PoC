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
    add_link_options(-static)
endif()

# Global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Core simulation sources
set(SIM_CORE_SOURCES
    src/core/SimulationClock.cpp
    src/core/Logger.cpp
    src/simulation/VirtualTarget.cpp
    src/simulation/GroundTruthGenerator.cpp
    src/simulation/SimulationEngine.cpp
    src/data/ScenarioLoader.cpp
    src/metrics/MetricsCollector.cpp
)

add_library(sim_core STATIC ${SIM_CORE_SOURCES})
target_include_directories(sim_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# S-001 Runner executable
add_executable(jordanian_sim_s001 src/main.cpp)
target_link_libraries(jordanian_sim_s001 PRIVATE sim_core)

# Tests executable
option(ENABLE_SIM_TESTING "Build simulation unit tests" ON)
if (ENABLE_SIM_TESTING)
    enable_testing()
    add_executable(jordanian_sim_tests tests/test_main.cpp)
    target_link_libraries(jordanian_sim_tests PRIVATE sim_core)
    add_test(NAME AllUnitTests COMMAND jordanian_sim_tests)
endif()
