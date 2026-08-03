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
    src/evidence/artifact_registry.cpp
    src/evidence/json.cpp
    src/evidence/json_import.cpp
    src/evidence/packet_range_validation.cpp
    src/evidence/packet_registry.cpp
    src/evidence/registry.cpp
    src/exe/evidence_address_resolver.cpp
    src/exe/known_target.cpp
    src/exe/pe_image.cpp
    src/exe/pe_reader.cpp
    src/formats/container.cpp
    src/formats/container_parser_registry.cpp
    src/formats/dca.cpp
    src/formats/dca_binary.cpp
    src/formats/hits.cpp
    src/formats/hits_binary.cpp
    src/formats/lig2.cpp
    src/formats/lig2_binary.cpp
    src/formats/stage_txt.cpp
    src/formats/stage_txt_binary.cpp
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
    src/integration/executable_context.cpp
    src/integration/executable_evidence.cpp
    src/integration/executable_workspace_manifest.cpp
    src/integration/format_registry.cpp
    src/integration/project_graph.cpp
    src/integration/project_graph_manifest.cpp
    src/integration/project_workspace.cpp
    src/integration/project_workspace_evidence.cpp
    src/integration/project_workspace_executable.cpp
    src/integration/project_workspace_parser.cpp
    src/integration/resource_analyzer.cpp
    src/integration/resource_workspace.cpp
    src/integration/resource_workspace_executable.cpp
    src/integration/resource_workspace_parser.cpp
    src/integration/stage_view_consistency.cpp
    src/integration/stage_workspace_manifest.cpp
    src/integration/tool_registry.cpp
    src/integration/workspace_event.cpp
    src/integration/workspace_manifest.cpp
    src/item/runtime_request.cpp
    src/item/workspace_manifest.cpp
    src/item/workspace_view.cpp
    src/modviz/scene_workspace_view.cpp
    src/patch/guarded_patch.cpp
    src/profiles/dmc3/known_targets.cpp
    src/profiles/dmc3/stage_resources.cpp
    src/profiles/dmc3/stage_table.cpp
    src/profiles/dmc3/stage_workspace_builder.cpp
    src/stageops/workspace_view.cpp
    src/validation/item_runtime_plan.cpp
)

add_library(DMCRengine::Core ALIAS dmc_rengine_core)

target_include_directories(dmc_rengine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(dmc_rengine_core PUBLIC cxx_std_20)
dmc_rengine_enable_warnings(dmc_rengine_core)

add_executable(dmc-rengine
    src/cli/main.cpp
    src/cli/integration_commands.cpp
    src/cli/integration_dispatch.cpp
    src/cli/stage_commands.cpp
)
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
    dmc_rengine_configure_test(
        dmc_rengine_evidence_packet_project_tests
        tests/evidence_packet_project_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_evidence_packet_range_tests
        tests/evidence_packet_range_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_public_evidence_project_tests
        tests/public_evidence_project_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_reader_tests tests/binary_reader_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_document_tests tests/binary_document_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_fields_tests tests/binary_fields_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_binary_manifest_tests tests/binary_manifest_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_hits_tests tests/hits_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_hits_binary_tests tests/hits_binary_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_dca_tests tests/dca_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_lig2_tests tests/lig2_tests.cpp)
    dmc_rengine_configure_test(dmc_rengine_stage_txt_tests tests/stage_txt_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_resource_analyzer_tests
        tests/resource_analyzer_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_executable_workspace_manifest_tests
        tests/executable_workspace_manifest_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_evidence_address_resolver_tests
        tests/evidence_address_resolver_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_integration_workspace_tests
        tests/integration_workspace_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_workspace_manifest_tests
        tests/workspace_manifest_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_project_graph_tests
        tests/project_graph_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_project_graph_manifest_tests
        tests/project_graph_manifest_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_project_workspace_tests
        tests/project_workspace_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_stage_workspace_manifest_tests
        tests/stage_workspace_manifest_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_dmc3_stage_workspace_builder_tests
        tests/dmc3_stage_workspace_builder_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_stage_tool_views_tests
        tests/stage_tool_views_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_item_workspace_tests
        tests/item_workspace_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_item_runtime_graph_tests
        tests/item_runtime_graph_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_item_workspace_manifest_tests
        tests/item_workspace_manifest_tests.cpp)
    dmc_rengine_configure_test(
        dmc_rengine_item_runtime_validation_plan_tests
        tests/item_runtime_validation_plan_tests.cpp)
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
    add_test(
        NAME dmc_rengine_evidence_packet_project_tests
        COMMAND dmc_rengine_evidence_packet_project_tests)
    add_test(
        NAME dmc_rengine_evidence_packet_range_tests
        COMMAND dmc_rengine_evidence_packet_range_tests)
    add_test(
        NAME dmc_rengine_public_evidence_project_tests
        COMMAND dmc_rengine_public_evidence_project_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json)
    add_test(NAME dmc_rengine_binary_reader_tests COMMAND dmc_rengine_binary_reader_tests)
    add_test(NAME dmc_rengine_binary_document_tests COMMAND dmc_rengine_binary_document_tests)
    add_test(NAME dmc_rengine_binary_fields_tests COMMAND dmc_rengine_binary_fields_tests)
    add_test(NAME dmc_rengine_binary_manifest_tests COMMAND dmc_rengine_binary_manifest_tests)
    add_test(NAME dmc_rengine_hits_tests COMMAND dmc_rengine_hits_tests)
    add_test(NAME dmc_rengine_hits_binary_tests COMMAND dmc_rengine_hits_binary_tests)
    add_test(NAME dmc_rengine_dca_tests COMMAND dmc_rengine_dca_tests)
    add_test(NAME dmc_rengine_lig2_tests COMMAND dmc_rengine_lig2_tests)
    add_test(NAME dmc_rengine_stage_txt_tests COMMAND dmc_rengine_stage_txt_tests)
    add_test(
        NAME dmc_rengine_resource_analyzer_tests
        COMMAND dmc_rengine_resource_analyzer_tests)
    add_test(
        NAME dmc_rengine_executable_workspace_manifest_tests
        COMMAND dmc_rengine_executable_workspace_manifest_tests)
    add_test(
        NAME dmc_rengine_evidence_address_resolver_tests
        COMMAND dmc_rengine_evidence_address_resolver_tests)
    add_test(
        NAME dmc_rengine_integration_workspace_tests
        COMMAND dmc_rengine_integration_workspace_tests)
    add_test(
        NAME dmc_rengine_workspace_manifest_tests
        COMMAND dmc_rengine_workspace_manifest_tests)
    add_test(
        NAME dmc_rengine_project_graph_tests
        COMMAND dmc_rengine_project_graph_tests)
    add_test(
        NAME dmc_rengine_project_graph_manifest_tests
        COMMAND dmc_rengine_project_graph_manifest_tests)
    add_test(
        NAME dmc_rengine_project_workspace_tests
        COMMAND dmc_rengine_project_workspace_tests)
    add_test(
        NAME dmc_rengine_stage_workspace_manifest_tests
        COMMAND dmc_rengine_stage_workspace_manifest_tests)
    add_test(
        NAME dmc_rengine_dmc3_stage_workspace_builder_tests
        COMMAND dmc_rengine_dmc3_stage_workspace_builder_tests)
    add_test(
        NAME dmc_rengine_stage_tool_views_tests
        COMMAND dmc_rengine_stage_tool_views_tests)
    add_test(
        NAME dmc_rengine_item_workspace_tests
        COMMAND dmc_rengine_item_workspace_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)
    add_test(
        NAME dmc_rengine_item_runtime_graph_tests
        COMMAND dmc_rengine_item_runtime_graph_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)
    add_test(
        NAME dmc_rengine_item_workspace_manifest_tests
        COMMAND dmc_rengine_item_workspace_manifest_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)
    add_test(
        NAME dmc_rengine_item_runtime_validation_plan_tests
        COMMAND dmc_rengine_item_runtime_validation_plan_tests)
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
    add_test(
        NAME dmc_rengine_item_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)
    add_test(NAME dmc_rengine_cli_help COMMAND dmc-rengine help)
    add_test(NAME dmc_rengine_cli_list_tools COMMAND dmc-rengine list-tools)
    add_test(NAME dmc_rengine_cli_list_formats COMMAND dmc-rengine list-formats)
    add_test(NAME dmc_rengine_cli_integration_status COMMAND dmc-rengine integration-status)
endif()
