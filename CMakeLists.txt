cmake_minimum_required(VERSION 3.25)

project(DMCRengine VERSION 0.2.0 LANGUAGES CXX)

option(DMC_RENGINE_BUILD_TESTS "Build DMC Rengine tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(dmc_rengine_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
endfunction()

function(dmc_rengine_configure_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE DMCRengine::Core)
    dmc_rengine_enable_warnings(${target})

    # Force a test-only assertion shim before any source includes. It always
    # evaluates expressions and reports file/line in both Debug and Release.
    if(MSVC)
        target_compile_options(
            ${target}
            PRIVATE /FI${CMAKE_CURRENT_SOURCE_DIR}/tests/test_support.hpp)
    else()
        target_compile_options(
            ${target}
            PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_support.hpp)
    endif()
endfunction()

add_library(dmc_rengine_core
    src/binary/reader.cpp
    src/core/sha256.cpp
    src/core/version.cpp
    src/evidence/json.cpp
    src/evidence/registry.cpp
    src/exe/pe_image.cpp
    src/exe/pe_reader.cpp
    src/gdspaces/local_directory_source.cpp
    src/gdspaces/open_router.cpp
    src/gdspaces/resource_graph.cpp
    src/gdspaces/resource_id.cpp
    src/gdspaces/source_registry.cpp
    src/gdspaces/stage_bundle.cpp
    src/gdspaces/working_copy.cpp
    src/patch/guarded_patch.cpp
)

add_library(DMCRengine::Core ALIAS dmc_rengine_core)

target_include_directories(dmc_rengine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(dmc_rengine_core PUBLIC cxx_std_20)
dmc_rengine_enable_warnings(dmc_rengine_core)

add_executable(dmc-rengine src/cli/main.cpp)
target_link_libraries(dmc-rengine PRIVATE DMCRengine::Core)
dmc_rengine_enable_warnings(dmc-rengine)

include(CTest)
if(BUILD_TESTING AND DMC_RENGINE_BUILD_TESTS)
    dmc_rengine_configure_test(dmc_rengine_tests tests/core_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_sha256_tests tests/sha256_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_patch_tests tests/patch_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_evidence_json_tests tests/evidence_json_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_reader_tests tests/binary_reader_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_working_copy_tests tests/working_copy_tests.cpp)

    add_test(NAME dmc_rengine_core_tests COMMAND dmc_rengine_tests)
    add_test(NAME dmc_rengine_sha256_tests COMMAND dmc_rengine_sha256_tests)
    add_test(NAME dmc_rengine_patch_tests COMMAND dmc_rengine_patch_tests)
    add_test(NAME dmc_rengine_evidence_json_tests COMMAND dmc_rengine_evidence_json_tests)
    add_test(NAME dmc_rengine_binary_reader_tests COMMAND dmc_rengine_binary_reader_tests)
    add_test(NAME dmc_rengine_working_copy_tests COMMAND dmc_rengine_working_copy_tests)
endif()
