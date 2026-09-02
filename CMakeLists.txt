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

function(dmc_rengine_add_simple_test stem)
    set(target "dmc_rengine_${stem}_tests")
    dmc_rengine_configure_test(${target} "tests/${stem}_tests.cpp")
    add_test(NAME ${target} COMMAND ${target})
endfunction()

file(GLOB_RECURSE DMC_RENGINE_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER DMC_RENGINE_CORE_SOURCES EXCLUDE REGEX "/src/cli/")

add_library(dmc_rengine_core ${DMC_RENGINE_CORE_SOURCES})
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
    src/cli/exe_acquisition_commands.cpp
    src/cli/integration_commands.cpp
    src/cli/integration_dispatch.cpp
    src/cli/dmc3_build_authority_commands.cpp
    src/cli/dmc3_l1_closure_commands.cpp
    src/cli/dmc3_overlay_commands.cpp
    src/cli/dmc3_retail_acquisition_commands.cpp
    src/cli/nbz_copy_commands.cpp
    src/cli/relative_slot_commands.cpp
    src/cli/hits_commands.cpp
    src/cli/stage_commands.cpp
)
target_link_libraries(dmc-rengine PRIVATE DMCRengine::Core)
dmc_rengine_enable_warnings(dmc-rengine)

include(CTest)
if(BUILD_TESTING AND DMC_RENGINE_BUILD_TESTS)
    dmc_rengine_configure_test(dmc_rengine_tests tests/core_tests.cpp)
    add_test(NAME dmc_rengine_core_tests COMMAND dmc_rengine_tests)

    set(DMC_RENGINE_SIMPLE_TESTS
        raw_deflate
        nbz_zip_source
        nbz_zip_serialization
        nbz_zip_artifact_binding
        nbz_zip_artifact_member
        nbz_zip_repacker_publication
        nbz_overlay_writer
        next_volume_overlay_resolution
        resource_path_normalizer
        tagged_record_classification
        texture_pack_container
        scm_structural
        mod_structural
        effect_and_shadow_structural
        slot_name_manifest
        mot_structural
        effect_pack_structural
        named_but_not_a_container
        manifest_names_and_slot_numbering
        animation_formats_and_walk
        content_tag_census
        demo_script_naming
        classifier_registry_agreement
        effect_pack_names
        slot_zero_manifest_authority
        texture_partial_mip_chain
        no_replace_publication
        external_index_naming
        index_sidecar_manifest
        container_index_probe
        resource_lookup_policy
        recovered_exe_contract
        source_lookup
        volume_bootstrap_policy
        runtime_resource_resolver
        runtime_naming_bridge
        physical_provider_receipt
        retail_member_acquisition
        loose_container_list
        relative_slot_writer
        relative_slot_packed_reflow_writer
        relative_slot_path_reflow_writer
        gdspaces_l1_full_a2z_roundtrip
        dds_profile
        texture_slot_framing
        texture_slot_packed_reflow_writer
        texture_slot_runtime_materialization
        texture_slot_index_name_authority
        runtime_synth_relative_slot_writer
        runtime_synth_result_integrity
        runtime_synth_format_registry
        nested_relative_slot_reintegrator
        index_name_nested_reopen
        sha256
        patch
        json_parser
        json_number_portability
        evidence_json
        evidence_json_import
        evidence_packet_project
        evidence_packet_range
        binary_reader
        binary_document
        binary_fields
        binary_manifest
        binary_analysis
        hits
        hits_writer
        hits_spatial_compare
        hits_spatial_match
        hits_binary
        dca
        lig2
        stage_txt
        resource_analyzer
        resource_analyzer_structural_formats
        executable_workspace_manifest
        evidence_address_resolver
        exe_byte_window
        exe_byte_window_receipt
        process_memory_window
        process_memory_window_receipt
        integration_workspace
        workspace_manifest
        project_graph
        project_graph_manifest
        project_workspace
        stage_workspace_manifest
        dmc3_stage_workspace_builder
        stage_tool_views
        item_runtime_validation_plan
        item_runtime_validation_graph
        item_runtime_patch_compiler
        copy_executor
        source_modification
        source_integration_project
        source_integration_graph
        source_integration_manifest
        source_integration_evidence_gate
        custom_build
        custom_build_graph
        custom_build_import
        custom_build_manifest
        custom_build_reopen
        pc_save_file
        working_copy
        byte_provenance
        resource_id_canonical
        index_name_authority
        index_name_evidence_persistence
        embedded_name_evidence
        effect_stored_name_evidence
        resource_naming_identity
        companion_index_locator
        legacy_index_replay
        dmc3_naming_pipeline
        dmc3_runtime_content_tag_provenance
        pac
        pnst
        dmc3_container_parsers
        container_tree_expander
        container_expander_identity
        classifier
        known_target
        executable_authority
        stage_table_descriptor
        stage_resource_matcher
        stage_bundle_assembler
        container_foundation
    )
    foreach(stem IN LISTS DMC_RENGINE_SIMPLE_TESTS)
        dmc_rengine_add_simple_test(${stem})
    endforeach()

    dmc_rengine_configure_test(
        dmc_rengine_relative_slot_cli_tests
        tests/relative_slot_cli_tests.cpp)
    target_sources(
        dmc_rengine_relative_slot_cli_tests
        PRIVATE src/cli/relative_slot_commands.cpp)
    target_include_directories(
        dmc_rengine_relative_slot_cli_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cli)
    add_test(
        NAME dmc_rengine_relative_slot_cli_tests
        COMMAND dmc_rengine_relative_slot_cli_tests)

    dmc_rengine_configure_test(
        dmc_rengine_public_evidence_project_tests
        tests/public_evidence_project_tests.cpp)
    add_test(
        NAME dmc_rengine_public_evidence_project_tests
        COMMAND dmc_rengine_public_evidence_project_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json)

    dmc_rengine_configure_test(
        dmc_rengine_item_workspace_tests
        tests/item_workspace_tests.cpp)
    add_test(
        NAME dmc_rengine_item_workspace_tests
        COMMAND dmc_rengine_item_workspace_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)

    dmc_rengine_configure_test(
        dmc_rengine_item_runtime_graph_tests
        tests/item_runtime_graph_tests.cpp)
    add_test(
        NAME dmc_rengine_item_runtime_graph_tests
        COMMAND dmc_rengine_item_runtime_graph_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)

    dmc_rengine_configure_test(
        dmc_rengine_item_workspace_manifest_tests
        tests/item_workspace_manifest_tests.cpp)
    add_test(
        NAME dmc_rengine_item_workspace_manifest_tests
        COMMAND dmc_rengine_item_workspace_manifest_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)

    dmc_rengine_configure_test(
        dmc_rengine_reverse_authority_manifest_tests
        tests/reverse_authority_manifest_tests.cpp)
    add_test(
        NAME dmc_rengine_reverse_authority_manifest_tests
        COMMAND dmc_rengine_reverse_authority_manifest_tests
                ${CMAKE_CURRENT_SOURCE_DIR}/data/registries/dmc3-reverse-authorities.v1.json)

    add_test(
        NAME dmc_rengine_public_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/known-targets/dmc3-hdc-phase12.evidence.json)
    add_test(
        NAME dmc_rengine_item_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/items/dmc3-item-runtime.evidence.json)
    add_test(
        NAME dmc_rengine_save_pass31_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/save/dmc3-pc-save-pass31.evidence.json)
    add_test(
        NAME dmc_rengine_save_pass32_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/save/dmc3-pc-save-pass32.evidence.json)
    add_test(NAME dmc_rengine_cli_help COMMAND dmc-rengine help)
    add_test(NAME dmc_rengine_cli_list_tools COMMAND dmc-rengine list-tools)
    add_test(NAME dmc_rengine_cli_list_formats COMMAND dmc-rengine list-formats)
    add_test(NAME dmc_rengine_cli_integration_status COMMAND dmc-rengine integration-status)
    add_test(
        NAME dmc_rengine_cli_preflight_dmc3_game_test_usage
        COMMAND dmc-rengine preflight-dmc3-game-test)
    set_tests_properties(
        dmc_rengine_cli_preflight_dmc3_game_test_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_preflight_dmc3_game_test_unknown
        COMMAND ${CMAKE_COMMAND}
                -DDMC_RENGINE_CLI=$<TARGET_FILE:dmc-rengine>
                -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/dmc3-game-test-preflight
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/dmc3_game_test_preflight_cli_test.cmake)
    add_test(
        NAME dmc_rengine_cli_verify_dmc3_l1_authoring_usage
        COMMAND dmc-rengine verify-dmc3-l1-authoring)
    set_tests_properties(
        dmc_rengine_cli_verify_dmc3_l1_authoring_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_build_dmc3_overlay_usage
        COMMAND dmc-rengine build-dmc3-overlay)
    set_tests_properties(
        dmc_rengine_cli_build_dmc3_overlay_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_build_dmc3_overlay_artifact
        COMMAND ${CMAKE_COMMAND}
                -DDMC_RENGINE_CLI=$<TARGET_FILE:dmc-rengine>
                -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/dmc3-overlay-cli-test
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/build_dmc3_overlay_cli_test.cmake)
    add_test(
        NAME dmc_rengine_cli_extract_dmc3_retail_member_usage
        COMMAND dmc-rengine extract-dmc3-retail-member)
    set_tests_properties(
        dmc_rengine_cli_extract_dmc3_retail_member_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_extract_dmc3_retail_member
        COMMAND ${CMAKE_COMMAND}
                -DDMC_RENGINE_CLI=$<TARGET_FILE:dmc-rengine>
                -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/dmc3-retail-member-acquisition-test
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/extract_dmc3_retail_member_cli_test.cmake)
    add_test(
        NAME dmc_rengine_cli_build_nbz_copy_usage
        COMMAND dmc-rengine build-nbz-copy)
    set_tests_properties(
        dmc_rengine_cli_build_nbz_copy_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_build_nbz_copy
        COMMAND ${CMAKE_COMMAND}
                -DDMC_RENGINE_CLI=$<TARGET_FILE:dmc-rengine>
                -DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/nbz-copy-cli-test
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/build_nbz_copy_cli_test.cmake)
    add_test(
        NAME dmc_rengine_cli_rebuild_relative_slot_usage
        COMMAND dmc-rengine rebuild-relative-slot)
    set_tests_properties(
        dmc_rengine_cli_rebuild_relative_slot_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_extract_exe_window_usage
        COMMAND dmc-rengine extract-exe-window)
    set_tests_properties(
        dmc_rengine_cli_extract_exe_window_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_capture_exe_process_window_usage
        COMMAND dmc-rengine capture-exe-process-window)
    set_tests_properties(
        dmc_rengine_cli_capture_exe_process_window_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_compare_hits_spatial_usage
        COMMAND dmc-rengine compare-hits-spatial)
    set_tests_properties(
        dmc_rengine_cli_compare_hits_spatial_usage
        PROPERTIES WILL_FAIL TRUE)
endif()