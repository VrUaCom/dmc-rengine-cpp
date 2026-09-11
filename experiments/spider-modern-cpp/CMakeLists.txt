cmake_minimum_required(VERSION 3.25)

project(DMCRengineSpiderModernCppExperiment LANGUAGES CXX)

set(SPIDER_EXPERIMENT_CXX "23" CACHE STRING
    "C++ language level for the Spider modern-C++ experiment (23 or 26)")
set_property(CACHE SPIDER_EXPERIMENT_CXX PROPERTY STRINGS 23 26)

if(NOT SPIDER_EXPERIMENT_CXX STREQUAL "23" AND
   NOT SPIDER_EXPERIMENT_CXX STREQUAL "26")
    message(FATAL_ERROR
        "SPIDER_EXPERIMENT_CXX must be 23 or 26, got '${SPIDER_EXPERIMENT_CXX}'")
endif()

# Keep the repository's canonical C++20 core unchanged. This experiment builds
# it as-is, then compiles only the probe executable at the requested newer
# language level.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(DMC_RENGINE_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    "${CMAKE_CURRENT_BINARY_DIR}/dmc-rengine-core")

add_executable(
    dmc-rengine-spider-mod-skin-modern
    spider_mod_skin_modern.cpp)

target_link_libraries(
    dmc-rengine-spider-mod-skin-modern
    PRIVATE DMCRengine::Core)

set_target_properties(
    dmc-rengine-spider-mod-skin-modern
    PROPERTIES
        CXX_STANDARD ${SPIDER_EXPERIMENT_CXX}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(
        dmc-rengine-spider-mod-skin-modern
        PRIVATE /O2 /W4 /permissive-)
else()
    target_compile_options(
        dmc-rengine-spider-mod-skin-modern
        PRIVATE -O3 -Wall -Wextra -Wpedantic -Wconversion)
endif()
