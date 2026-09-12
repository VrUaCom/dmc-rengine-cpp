# Canonical portable read-only slice for platform viewers.
#
# This file is intentionally self-contained so consumers can include it from a
# pinned dmc-rengine-cpp checkout/submodule without importing the full Rengine
# CLI, authoring, reverse-analysis, or development-tool graph.

if(TARGET dmc_rengine_reader_core)
    return()
endif()

get_filename_component(
    DMC_RENGINE_READER_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/.."
    ABSOLUTE)

set(DMC_RENGINE_READER_CORE_SOURCES
    "${DMC_RENGINE_READER_ROOT}/src/binary/reader.cpp"

    # Portable texture reader primitives. DMC-specific descriptor/PTX framing
    # stays in Rengine; platform shells never parse +0x38/+0x64/0x70 offsets.
    "${DMC_RENGINE_READER_ROOT}/src/codecs/dds_bc.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/profiles/dmc3/texture_slot_framing.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/profiles/dmc3/texture_slot_framing_compat.cpp"

    # Canonical EVT structural reader used by EventTbl inspection surfaces.
    "${DMC_RENGINE_READER_ROOT}/src/formats/evt.cpp"

    # Canonical MOD read-side slice already consumed by Native Reader.
    "${DMC_RENGINE_READER_ROOT}/src/formats/mod_skin.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/mod/transform_domain.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/mod/world_transform.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/mod.cpp"

    # Canonical SCM structural/read-side slice.
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm_layout.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm_topology.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm_validation.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm_transform.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm_hierarchy.cpp"
    "${DMC_RENGINE_READER_ROOT}/src/formats/scm.cpp"
)

add_library(dmc_rengine_reader_core STATIC
    ${DMC_RENGINE_READER_CORE_SOURCES})
add_library(DMCRengine::ReaderCore ALIAS dmc_rengine_reader_core)

target_include_directories(dmc_rengine_reader_core
    PUBLIC
        "${DMC_RENGINE_READER_ROOT}/include")

target_compile_features(dmc_rengine_reader_core PUBLIC cxx_std_20)

# ReaderCore is linked into Android/iOS shared/framework binaries as well as
# desktop executables and WASM, so keep the static archive PIC-safe.
set_target_properties(dmc_rengine_reader_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON)

if(COMMAND dmc_rengine_enable_warnings)
    dmc_rengine_enable_warnings(dmc_rengine_reader_core)
endif()
