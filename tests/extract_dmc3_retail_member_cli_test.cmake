if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(EXE_DIR "${TEST_ROOT}/game")
set(DATA_DIR "${EXE_DIR}/data/dmc3")
set(BASE_OUT "${TEST_ROOT}/base-out")
set(OVERLAY_OUT "${TEST_ROOT}/overlay-out")
set(ACQUIRE_DIR "${TEST_ROOT}/acquired")
file(MAKE_DIRECTORY "${DATA_DIR}" "${BASE_OUT}" "${OVERLAY_OUT}" "${ACQUIRE_DIR}")

set(BASE_AUTHORED "${TEST_ROOT}/base-resource.pac")
set(MOD_AUTHORED "${TEST_ROOT}/modified-resource.pac")
file(WRITE "${BASE_AUTHORED}" "ORIGINAL-RETAIL-MEMBER")
file(WRITE "${MOD_AUTHORED}" "MODIFIED-HIGHER-VOLUME-MEMBER")

set(GAME_REQUEST "obj\\retail-test.pac")

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" build-dmc3-overlay
            "${EXE_DIR}" "${GAME_REQUEST}" "${BASE_AUTHORED}" "${BASE_OUT}"
    RESULT_VARIABLE BASE_RESULT
    OUTPUT_VARIABLE BASE_STDOUT
    ERROR_VARIABLE BASE_STDERR)
if(NOT BASE_RESULT EQUAL 0)
    message(FATAL_ERROR
        "base overlay build failed (${BASE_RESULT})\nstdout:\n${BASE_STDOUT}\nstderr:\n${BASE_STDERR}")
endif()
file(COPY_FILE "${BASE_OUT}/DMC3-0.nbz" "${DATA_DIR}/DMC3-0.nbz" ONLY_IF_DIFFERENT)

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" build-dmc3-overlay
            "${EXE_DIR}" "${GAME_REQUEST}" "${MOD_AUTHORED}" "${OVERLAY_OUT}"
    RESULT_VARIABLE OVERLAY_RESULT
    OUTPUT_VARIABLE OVERLAY_STDOUT
    ERROR_VARIABLE OVERLAY_STDERR)
if(NOT OVERLAY_RESULT EQUAL 0)
    message(FATAL_ERROR
        "higher-volume overlay build failed (${OVERLAY_RESULT})\nstdout:\n${OVERLAY_STDOUT}\nstderr:\n${OVERLAY_STDERR}")
endif()
file(COPY_FILE "${OVERLAY_OUT}/DMC3-1.nbz" "${DATA_DIR}/DMC3-1.nbz" ONLY_IF_DIFFERENT)

set(EXTRACTED "${ACQUIRE_DIR}/retail-test.pac")
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" extract-dmc3-retail-member
            "${EXE_DIR}" "${GAME_REQUEST}" "${EXTRACTED}"
    RESULT_VARIABLE ACQUIRE_RESULT
    OUTPUT_VARIABLE ACQUIRE_STDOUT
    ERROR_VARIABLE ACQUIRE_STDERR)
if(NOT ACQUIRE_RESULT EQUAL 0)
    message(FATAL_ERROR
        "retail acquisition failed (${ACQUIRE_RESULT})\nstdout:\n${ACQUIRE_STDOUT}\nstderr:\n${ACQUIRE_STDERR}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${EXTRACTED}" "${MOD_AUTHORED}"
    RESULT_VARIABLE COMPARE_RESULT)
if(NOT COMPARE_RESULT EQUAL 0)
    message(FATAL_ERROR "extracted member is not the higher-volume modified resource")
endif()

set(RECEIPT "${EXTRACTED}.receipt.json")
if(NOT EXISTS "${RECEIPT}")
    message(FATAL_ERROR "acquisition receipt was not produced")
endif()
file(READ "${RECEIPT}" RECEIPT_TEXT)
string(FIND "${RECEIPT_TEXT}" "\"selected_volume_index\": 1" VOLUME_POS)
if(VOLUME_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not identify higher volume 1")
endif()
string(FIND "${RECEIPT_TEXT}" "GDataX360.afs/retail-test.pac" MEMBER_POS)
if(MEMBER_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not preserve resolved runtime archive member identity")
endif()
string(FIND "${RECEIPT_TEXT}" "\"evidence_class\": \"direct-retail-member-acquisition\"" CLASS_POS)
if(CLASS_POS EQUAL -1)
    message(FATAL_ERROR "receipt evidence class is missing")
endif()

# Publication is no-clobber: repeating the exact acquisition must fail rather
# than silently replacing the previous evidence output/receipt.
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" extract-dmc3-retail-member
            "${EXE_DIR}" "${GAME_REQUEST}" "${EXTRACTED}"
    RESULT_VARIABLE REPEAT_RESULT
    OUTPUT_QUIET
    ERROR_QUIET)
if(REPEAT_RESULT EQUAL 0)
    message(FATAL_ERROR "repeat acquisition unexpectedly overwrote an existing output")
endif()
