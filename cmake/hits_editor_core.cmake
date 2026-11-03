# Portable authoring/view slice. Consumed by standalone shells and Rengine.
if(TARGET dmc_rengine_hits_editor_core)
    return()
endif()
include(${CMAKE_CURRENT_LIST_DIR}/reader_core.cmake)
get_filename_component(HITS_RENGINE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
add_library(dmc_rengine_hits_editor_core STATIC
    ${HITS_RENGINE_ROOT}/src/formats/hits.cpp
    ${HITS_RENGINE_ROOT}/src/hits/writer.cpp
    ${HITS_RENGINE_ROOT}/src/hits/editor.cpp
    ${HITS_RENGINE_ROOT}/src/hits/scm_import.cpp
    ${HITS_RENGINE_ROOT}/src/hits/standalone_session.cpp
    ${HITS_RENGINE_ROOT}/src/hits/viewport.cpp)
add_library(DMCRengine::HitsEditorCore ALIAS dmc_rengine_hits_editor_core)
target_link_libraries(dmc_rengine_hits_editor_core PUBLIC DMCRengine::ReaderCore)
target_compile_features(dmc_rengine_hits_editor_core PUBLIC cxx_std_20)
set_target_properties(dmc_rengine_hits_editor_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
