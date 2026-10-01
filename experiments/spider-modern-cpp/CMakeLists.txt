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
# it as-is, then compiles only the probe executables at the requested newer
# language level.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(DMC_RENGINE_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_LIST_DIR}/../.."
    "${CMAKE_CURRENT_BINARY_DIR}/dmc-rengine-core")

function(spider_modern_experiment target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE DMCRengine::Core)
    set_target_properties(
        ${target}
        PROPERTIES
            CXX_STANDARD ${SPIDER_EXPERIMENT_CXX}
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)

    if(MSVC)
        target_compile_options(${target} PRIVATE /O2 /W4 /permissive-)
    else()
        target_compile_options(
            ${target}
            PRIVATE -O3 -Wall -Wextra -Wpedantic -Wconversion)
    endif()
endfunction()

spider_modern_experiment(
    dmc-rengine-spider-mod-skin-modern
    spider_mod_skin_modern.cpp)

spider_modern_experiment(
    dmc-rengine-spider-mod-skin-trifamily
    spider_mod_skin_trifamily.cpp)
