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

    # The current lightweight tests use assert() as the check primitive.
    # Release configurations define NDEBUG, which removes both the check and
    # the evaluated expression. Undefine it at compiler level for every test.
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endfunction()

add_library(dmc_rengine_core
    src/binary/document.cpp
    src/binary/manifest.cpp
    src/binary/reader.cpp
    src/core/json.cpp
    src/core/sha256.cpp
    src/core/version.cpp
    src/evidence/json.cpp
    src/evidence/json_import.cpp
    src/evidence/registry.cpp
    src/exe/known_target.cpp
    src/exe/pe_image.cpp
    src/exe/pe_reader.cpp
    src/formats/container.cpp
    src/formats/container_parser_registry.cpp
    src/formats/hits.cpp
    src/formats/hits_binary.cpp
    src/formats/synthetic/slot_container_parser.cpp
    src/gdspaces/classifier.cpp
    src/gdspaces/container_expander.cpp
    src/gdspaces/local_directory_source.cpp
    src/gdspaces/open_router.cpp
    src/gdspaces/resource_graph.cpp
    src/gdspaces/resource_id.cpp
    src/gdspaces/source_registry.cpp
    src/gdspaces/stage_bundle.cpp
    src/gdspaces/stage_bundle_assembler.cpp
    src/gdspaces/working_copy.cpp
    src/integration/format_registry.cpp
    src/integration/resource_workspace.cpp
    src/integration/tool_registry.cpp
    src/integration/workspace_event.cpp
    src/integration/workspace_manifest.cpp
    src/patch/guarded_patch.cpp
    src/profiles/dmc3/known_targets.cpp
    src/profiles/dmc3/stage_resources.cpp
    src/profiles/dmc3/stage_table.cpp
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
    dmc_rengine_configure_test(dmc_rengine_json_parser_tests tests/json_parser_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_evidence_json_tests tests/evidence_json_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_evidence_json_import_tests
        tests/evidence_json_import_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_reader_tests tests/binary_reader_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_document_tests tests/binary_document_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_fields_tests tests/binary_fields_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_manifest_tests tests/binary_manifest_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_hits_tests tests/hits_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_hits_binary_tests tests/hits_binary_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_integration_workspace_tests
        tests/integration_workspace_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_workspace_manifest_tests
        tests/workspace_manifest_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_working_copy_tests tests/working_copy_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_classifier_tests tests/classifier_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_known_target_tests tests/known_target_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_stage_table_descriptor_tests
        tests/stage_table_descriptor_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_stage_resource_matcher_tests
        tests/stage_resource_matcher_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_stage_bundle_assembler_tests
        tests/stage_bundle_assembler_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_container_foundation_tests
        tests/container_foundation_tests.cpp)

    add_test(NAME dmc_rengine_core_tests COMMAND dmc_rengine_tests)
    add_test(NAME dmc_rengine_sha256_tests COMMAND dmc_rengine_sha256_tests)
    add_test(NAME dmc_rengine_patch_tests COMMAND dmc_rengine_patch_tests)
    add_test(NAME dmc_rengine_json_parser_tests COMMAND dmc_rengine_json_parser_tests)
    add_test(NAME dmc_rengine_evidence_json_tests COMMAND dmc_rengine_evidence_json_tests)
    add_test(
        NAME dmc_rengine_evidence_json_import_tests
        COMMAND dmc_rengine_evidence_json_import_tests)
    add_test(NAME dmc_rengine_binary_reader_tests COMMAND dmc_rengine_binary_reader_tests)
    add_test(NAME dmc_rengine_binary_document_tests COMMAND dmc_rengine_binary_document_tests)
    add_test(NAME dmc_rengine_binary_fields_tests COMMAND dmc_rengine_binary_fields_tests)
    add_test(NAME dmc_rengine_binary_manifest_tests COMMAND dmc_rengine_binary_manifest_tests)
    add_test(NAME dmc_rengine_hits_tests COMMAND dmc_rengine_hits_tests)
    add_test(NAME dmc_rengine_hits_binary_tests COMMAND dmc_rengine_hits_binary_tests)
    add_test(
        NAME dmc_rengine_integration_workspace_tests
        COMMAND dmc_rengine_integration_workspace_tests)
    add_test(
        NAME dmc_rengine_workspace_manifest_tests
        COMMAND dmc_rengine_workspace_manifest_tests)
    add_test(NAME dmc_rengine_working_copy_tests COMMAND dmc_rengine_working_copy_tests)
    add_test(NAME dmc_rengine_classifier_tests COMMAND dmc_rengine_classifier_tests)
    add_test(NAME dmc_rengine_known_target_tests COMMAND dmc_rengine_known_target_tests)
    add_test(
        NAME dmc_rengine_stage_table_descriptor_tests
        COMMAND dmc_rengine_stage_table_descriptor_tests)
    add_test(
        NAME dmc_rengine_stage_resource_matcher_tests
        COMMAND dmc_rengine_stage_resource_matcher_tests)
    add_test(
        NAME dmc_rengine_stage_bundle_assembler_tests
        COMMAND dmc_rengine_stage_bundle_assembler_tests)
    add_test(
        NAME dmc_rengine_container_foundation_tests
        COMMAND dmc_rengine_container_foundation_tests)
    add_test(
        NAME dmc_rengine_public_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json)
endif()
