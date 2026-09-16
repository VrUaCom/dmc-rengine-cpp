cmake_minimum_required(VERSION 3.25)

project(DMCRengine VERSION 0.2.0 LANGUAGES CXX)

option(DMC_RENGINE_BUILD_TESTS "Build DMC Rengine tests" ON)
option(DMC_RENGINE_BUILD_RUNTIME "Build the DMC Rengine runtime host layer" ON)
option(DMC_RENGINE_BUILD_CLI "Build the DMC Rengine command-line tool" ON)

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

function(dmc_rengine_add_runtime_test stem)
    set(target "dmc_rengine_${stem}_tests")
    add_executable(${target} "tests/${stem}_tests.cpp")
    target_link_libraries(${target} PRIVATE DMCRengine::Runtime)
    dmc_rengine_enable_warnings(${target})

    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()

    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(dmc_rengine_add_simple_test stem)
    set(target "dmc_rengine_${stem}_tests")
    dmc_rengine_configure_test(${target} "tests/${stem}_tests.cpp")
    add_test(NAME ${target} COMMAND ${target})
endfunction()

file(GLOB_RECURSE DMC_RENGINE_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER DMC_RENGINE_CORE_SOURCES EXCLUDE REGEX "/src/cli/")
# The runtime host layer is a separate target on its own C++ standard.
list(FILTER DMC_RENGINE_CORE_SOURCES EXCLUDE REGEX "/src/runtime/")
list(FILTER DMC_RENGINE_CORE_SOURCES EXCLUDE REGEX "/src/runtime_probe/")

add_library(dmc_rengine_core ${DMC_RENGINE_CORE_SOURCES})
add_library(DMCRengine::Core ALIAS dmc_rengine_core)

target_include_directories(dmc_rengine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(dmc_rengine_core PUBLIC cxx_std_20)
dmc_rengine_enable_warnings(dmc_rengine_core)

if(DMC_RENGINE_BUILD_CLI)
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
        src/cli/archive_key_census_commands.cpp
        src/cli/container_inspect_commands.cpp
        src/cli/mod_writer_corpus_commands.cpp
        src/cli/relative_slot_commands.cpp
        src/cli/hits_commands.cpp
        src/cli/stage_commands.cpp
    )
    target_link_libraries(dmc-rengine PRIVATE DMCRengine::Core)
    dmc_rengine_enable_warnings(dmc-rengine)
endif()

if(DMC_RENGINE_BUILD_RUNTIME)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/runtime.cmake")
endif()

include(CTest)
if(BUILD_TESTING AND DMC_RENGINE_BUILD_TESTS AND NOT DMC_RENGINE_BUILD_CLI)
    message(FATAL_ERROR
        "DMC_RENGINE_BUILD_TESTS=ON requires DMC_RENGINE_BUILD_CLI=ON: part of the suite drives the CLI.")
endif()

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
        resource_lookup_policy
        resource_id_canonical
        source_lookup
        volume_bootstrap_policy
        runtime_resource_resolver
        runtime_naming_bridge
        physical_provider_receipt
        loose_container_list
        relative_slot_writer
        relative_slot_packed_reflow_writer
        relative_slot_path_reflow_writer
        gdspaces_l1_full_a2z_roundtrip
        dds_profile
        gdata_legacy
        native_reader_texture
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
        model_family
        mod
        mod_writer
        mod_writer_corpus
        mod_authored_child_bridge
        mod_skin
        scm_writer
        scm_vertex_authoring
        scm_normal_authoring
        scm_uv_authoring
        scm_transform_inverse
        mot
        mot_key_decode
        stage_txt
        resource_analyzer
        resource_analyzer_structural_formats
        executable_workspace_manifest
        evidence_address_resolver
        exe_byte_window
        pe_directories
        x86_decoder
        string_table_scanner
        code_graph
        function_map
        rtti_scanner
        executable_report
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
        custom_build_manifest
        custom_build_reopen
        pc_save_file
        working_copy
        byte_provenance
        index_name_authority
        index_name_evidence_persistence
        embedded_name_evidence
        effect_stored_name_evidence
        effect_pack_records
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
        mot_structural
        text_resource_dialect
        scm
        scm_binary
        shw
        so
        so_graph
        so_link_table
        so_volume_table
        so_companion_correlation
        so_identity
        so_mod_binding
        mod_transform_domain
        format_registry_contract_coverage
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

    if(DMC_RENGINE_BUILD_RUNTIME)
        set(DMC_RENGINE_RUNTIME_TESTS
            runtime_status
            runtime_frame_clock
            runtime_headless_platform
            runtime_render_backend
            runtime_resource_bridge
            runtime_stage_host
            runtime_application
        )
        foreach(stem IN LISTS DMC_RENGINE_RUNTIME_TESTS)
            dmc_rengine_add_runtime_test(${stem})
        endforeach()
    endif()

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
        dmc_rengine_scm_occurrence_census_cli_tests
        tests/scm_occurrence_census_cli_tests.cpp)
    target_include_directories(
        dmc_rengine_scm_occurrence_census_cli_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cli)
    add_test(
        NAME dmc_rengine_scm_occurrence_census_cli_tests
        COMMAND dmc_rengine_scm_occurrence_census_cli_tests)

    dmc_rengine_configure_test(
        dmc_rengine_archive_key_census_cli_tests
        tests/archive_key_census_cli_tests.cpp)
    target_sources(
        dmc_rengine_archive_key_census_cli_tests
        PRIVATE src/cli/archive_key_census_commands.cpp)
    target_include_directories(
        dmc_rengine_archive_key_census_cli_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cli)
    add_test(
        NAME dmc_rengine_archive_key_census_cli_tests
        COMMAND dmc_rengine_archive_key_census_cli_tests)

    dmc_rengine_configure_test(
        dmc_rengine_container_inspect_cli_tests
        tests/container_inspect_cli_tests.cpp)
    target_sources(
        dmc_rengine_container_inspect_cli_tests
        PRIVATE src/cli/container_inspect_commands.cpp)
    target_include_directories(
        dmc_rengine_container_inspect_cli_tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cli)
    add_test(
        NAME dmc_rengine_container_inspect_cli_tests
        COMMAND dmc_rengine_container_inspect_cli_tests)

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
        NAME dmc_rengine_architecture_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-architecture.evidence.json)
    add_test(
        NAME dmc_rengine_constant_arguments_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-constant-arguments.evidence.json)
    add_test(
        NAME dmc_rengine_class_layout_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-class-layout.evidence.json)
    add_test(
        NAME dmc_rengine_virtual_dispatch_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-virtual-dispatch.evidence.json)
    add_test(
        NAME dmc_rengine_indexed_access_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-indexed-access.evidence.json)
    add_test(
        NAME dmc_rengine_table_extents_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-table-extents.evidence.json)
    add_test(
        NAME dmc_rengine_table_indices_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-table-indices.evidence.json)
    add_test(
        NAME dmc_rengine_table_linkage_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-table-linkage.evidence.json)
    add_test(
        NAME dmc_rengine_name_records_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-name-records.evidence.json)
    add_test(
        NAME dmc_rengine_name_tables_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-name-tables.evidence.json)
    add_test(
        NAME dmc_rengine_semantic_anchors_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-semantic-anchors.evidence.json)
    add_test(
        NAME dmc_rengine_code_recovery_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-code-recovery.evidence.json)
    add_test(
        NAME dmc_rengine_function_map_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-function-map.evidence.json)
    add_test(
        NAME dmc_rengine_structural_reverse_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/executable/dmc3-hdc-structural-reverse.evidence.json)
    add_test(
        NAME dmc_rengine_save_pass31_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/save/dmc3-pc-save-pass31.evidence.json)
    add_test(
        NAME dmc_rengine_save_pass32_evidence_validation
        COMMAND dmc-rengine validate-evidence
                ${CMAKE_CURRENT_SOURCE_DIR}/evidence/save/dmc3-pc-save-pass32.evidence.json)
    if(DMC_RENGINE_BUILD_RUNTIME AND TARGET dmc-rengine-runtime-probe)
        add_test(NAME dmc_rengine_runtime_probe_build_info
                 COMMAND dmc-rengine-runtime-probe)
        add_test(NAME dmc_rengine_runtime_probe_headless_frames
                 COMMAND dmc-rengine-runtime-probe --frames 120)
        add_test(NAME dmc_rengine_runtime_probe_rejects_unknown_argument
                 COMMAND dmc-rengine-runtime-probe --nope)
        set_tests_properties(dmc_rengine_runtime_probe_rejects_unknown_argument
                             PROPERTIES WILL_FAIL TRUE)
    endif()

    add_test(NAME dmc_rengine_cli_map_functions_usage COMMAND dmc-rengine map-functions)
    set_tests_properties(
        dmc_rengine_cli_map_functions_usage
        PROPERTIES WILL_FAIL TRUE)

    add_test(NAME dmc_rengine_cli_analyze_exe_usage COMMAND dmc-rengine analyze-exe)
    set_tests_properties(
        dmc_rengine_cli_analyze_exe_usage
        PROPERTIES WILL_FAIL TRUE)

    add_test(NAME dmc_rengine_cli_help COMMAND dmc-rengine help)
    add_test(NAME dmc_rengine_cli_list_tools COMMAND dmc-rengine list-tools)
    add_test(NAME dmc_rengine_cli_list_formats COMMAND dmc-rengine list-formats)
    add_test(NAME dmc_rengine_cli_integration_status COMMAND dmc-rengine integration-status)
    add_test(
        NAME dmc_rengine_cli_scm_set_vertex_position_usage
        COMMAND dmc-rengine scm-set-vertex-position)
    set_tests_properties(
        dmc_rengine_cli_scm_set_vertex_position_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_scm_set_vertex_normal_usage
        COMMAND dmc-rengine scm-set-vertex-normal)
    set_tests_properties(
        dmc_rengine_cli_scm_set_vertex_normal_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_scm_set_vertex_uv_usage
        COMMAND dmc-rengine scm-set-vertex-uv)
    set_tests_properties(
        dmc_rengine_cli_scm_set_vertex_uv_usage
        PROPERTIES WILL_FAIL TRUE)
    add_test(
        NAME dmc_rengine_cli_mod_writer_corpus_usage
        COMMAND dmc-rengine mod-writer-corpus)
    set_tests_properties(
        dmc_rengine_cli_mod_writer_corpus_usage
        PROPERTIES WILL_FAIL TRUE)
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
