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

# Recovered runtime registration stops at the first missing volume. A later
# in-domain file is diagnostic-only for reads and must not block or become a
# mount. DMC3-2 is intentionally absent; DMC3-3 is deliberately malformed so
# the test also proves it is never opened as a runtime archive.
file(WRITE "${DATA_DIR}/DMC3-3.nbz" "IGNORED-AFTER-FIRST-GAP")

# A product-discovered suffix outside signed %d runtime domain is likewise
# diagnostic-only and must not enter the mounted namespace.
file(WRITE "${DATA_DIR}/DMC3-2147483648.nbz" "IGNORED-OUTSIDE-RUNTIME-DOMAIN")

# Acquisition may never publish evidence back into the retail tree.
set(RETAIL_OUTPUT "${DATA_DIR}/forbidden-output.pac")
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" extract-dmc3-retail-member
            "${EXE_DIR}" "${GAME_REQUEST}" "${RETAIL_OUTPUT}"
    RESULT_VARIABLE RETAIL_OUTPUT_RESULT
    OUTPUT_QUIET
    ERROR_QUIET)
if(RETAIL_OUTPUT_RESULT EQUAL 0)
    message(FATAL_ERROR "retail-tree acquisition output unexpectedly succeeded")
endif()
if(EXISTS "${RETAIL_OUTPUT}" OR EXISTS "${RETAIL_OUTPUT}.receipt.json")
    message(FATAL_ERROR "failed retail-tree acquisition modified protected game data")
endif()

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
file(SHA256 "${EXTRACTED}" OUTPUT_SHA_BEFORE)

set(RECEIPT "${EXTRACTED}.receipt.json")
if(NOT EXISTS "${RECEIPT}")
    message(FATAL_ERROR "acquisition receipt was not produced")
endif()
file(READ "${RECEIPT}" RECEIPT_TEXT)
string(FIND "${RECEIPT_TEXT}" "\"schema_version\": 2" SCHEMA_POS)
if(SCHEMA_POS EQUAL -1)
    message(FATAL_ERROR "receipt schema version is not the artifact-bound contract")
endif()
string(FIND "${RECEIPT_TEXT}" "\"selected_volume_index\": 1" VOLUME_POS)
if(VOLUME_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not identify higher volume 1")
endif()
string(FIND "${RECEIPT_TEXT}" "GDataX360.afs/retail-test.pac" MEMBER_POS)
if(MEMBER_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not preserve resolved runtime archive member identity")
endif()
string(FIND "${RECEIPT_TEXT}" "\"first_missing_index\": 2" GAP_POS)
if(GAP_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not preserve first-gap runtime mount boundary")
endif()
string(FIND "${RECEIPT_TEXT}" "\"ignored_after_first_gap_count\": 1" AFTER_GAP_POS)
if(AFTER_GAP_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not preserve after-gap diagnostic evidence")
endif()
string(FIND "${RECEIPT_TEXT}" "\"ignored_outside_runtime_domain_count\": 1" OUTSIDE_DOMAIN_POS)
if(OUTSIDE_DOMAIN_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not preserve out-of-domain diagnostic evidence")
endif()
string(FIND "${RECEIPT_TEXT}" "\"evidence_class\": \"artifact-bound-retail-member-acquisition\"" CLASS_POS)
if(CLASS_POS EQUAL -1)
    message(FATAL_ERROR "artifact-bound receipt evidence class is missing")
endif()
string(FIND "${RECEIPT_TEXT}" "\"publication\": \"atomic-no-replace-per-artifact\"" PUBLICATION_POS)
if(PUBLICATION_POS EQUAL -1)
    message(FATAL_ERROR "receipt does not record no-replace publication policy")
endif()

# Repeating the exact acquisition must fail closed and preserve the original
# extracted artifact byte-for-byte.
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" extract-dmc3-retail-member
            "${EXE_DIR}" "${GAME_REQUEST}" "${EXTRACTED}"
    RESULT_VARIABLE REPEAT_RESULT
    OUTPUT_QUIET
    ERROR_QUIET)
if(REPEAT_RESULT EQUAL 0)
    message(FATAL_ERROR "repeat acquisition unexpectedly overwrote an existing output")
endif()
file(SHA256 "${EXTRACTED}" OUTPUT_SHA_AFTER)
if(NOT OUTPUT_SHA_AFTER STREQUAL OUTPUT_SHA_BEFORE)
    message(FATAL_ERROR "repeat acquisition changed the original evidence output")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
