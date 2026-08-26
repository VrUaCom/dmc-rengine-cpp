if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(BUILD_GAME "${TEST_ROOT}/build-game")
set(BUILD_DATA "${BUILD_GAME}/data/dmc3")
set(BASE_OUT "${TEST_ROOT}/base-out")
set(COPY_OUT "${TEST_ROOT}/copy-out")
set(VERIFY_GAME "${TEST_ROOT}/verify-game")
set(VERIFY_DATA "${VERIFY_GAME}/data/dmc3")
set(EXTRACT_OUT "${TEST_ROOT}/verify-extract")
file(MAKE_DIRECTORY
    "${BUILD_DATA}" "${BASE_OUT}" "${COPY_OUT}"
    "${VERIFY_DATA}" "${EXTRACT_OUT}")

set(ORIGINAL "${TEST_ROOT}/original.pac")
set(REPLACEMENT "${TEST_ROOT}/replacement.pac")
file(WRITE "${ORIGINAL}" "ORIGINAL-NBZ-COPY-MEMBER")
file(WRITE "${REPLACEMENT}" "REPLACEMENT-NBZ-COPY-MEMBER-WITH-SIZE-CHANGE")
set(GAME_REQUEST "obj\\copy-test.pac")

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" build-dmc3-overlay
            "${BUILD_GAME}" "${GAME_REQUEST}" "${ORIGINAL}" "${BASE_OUT}"
    RESULT_VARIABLE BUILD_RESULT
    OUTPUT_VARIABLE BUILD_STDOUT
    ERROR_VARIABLE BUILD_STDERR)
if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR
        "base NBZ build failed (${BUILD_RESULT})\nstdout:\n${BUILD_STDOUT}\nstderr:\n${BUILD_STDERR}")
endif()
set(SOURCE_NBZ "${BASE_OUT}/DMC3-0.nbz")
if(NOT EXISTS "${SOURCE_NBZ}")
    message(FATAL_ERROR "base NBZ artifact is missing")
endif()
file(SHA256 "${SOURCE_NBZ}" SOURCE_SHA_BEFORE)

set(OUTPUT_NBZ "${COPY_OUT}/modified.nbz")
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" build-nbz-copy
            "${SOURCE_NBZ}" 0 "${REPLACEMENT}" "${OUTPUT_NBZ}"
    RESULT_VARIABLE COPY_RESULT
    OUTPUT_VARIABLE COPY_STDOUT
    ERROR_VARIABLE COPY_STDERR)
if(NOT COPY_RESULT EQUAL 0)
    message(FATAL_ERROR
        "NBZ copy authoring failed (${COPY_RESULT})\nstdout:\n${COPY_STDOUT}\nstderr:\n${COPY_STDERR}")
endif()
if(NOT EXISTS "${OUTPUT_NBZ}")
    message(FATAL_ERROR "modified NBZ copy was not produced")
endif()
if(NOT EXISTS "${OUTPUT_NBZ}.receipt.json")
    message(FATAL_ERROR "NBZ copy authoring receipt was not produced")
endif()
file(SHA256 "${SOURCE_NBZ}" SOURCE_SHA_AFTER)
if(NOT SOURCE_SHA_AFTER STREQUAL SOURCE_SHA_BEFORE)
    message(FATAL_ERROR "immutable source NBZ changed during copy authoring")
endif()
file(SHA256 "${OUTPUT_NBZ}" OUTPUT_SHA_BEFORE)
if(OUTPUT_SHA_BEFORE STREQUAL SOURCE_SHA_BEFORE)
    message(FATAL_ERROR "size-changing replacement unexpectedly preserved source archive SHA")
endif()

file(READ "${OUTPUT_NBZ}.receipt.json" RECEIPT_TEXT)
string(FIND "${RECEIPT_TEXT}" "\"evidence_class\": \"artifact-bound-nbz-copy-authoring\"" CLASS_POS)
if(CLASS_POS EQUAL -1)
    message(FATAL_ERROR "NBZ copy receipt evidence class is missing")
endif()
string(FIND "${RECEIPT_TEXT}" "\"central_index\": 0" INDEX_POS)
if(INDEX_POS EQUAL -1)
    message(FATAL_ERROR "NBZ copy receipt target central index is missing")
endif()
string(FIND "${RECEIPT_TEXT}" "GDataX360.afs/copy-test.pac" MEMBER_POS)
if(MEMBER_POS EQUAL -1)
    message(FATAL_ERROR "NBZ copy receipt logical member identity is missing")
endif()

# No-replace behavior: rerunning at the same destination must fail and leave
# the already-verified artifact byte-identical.
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" build-nbz-copy
            "${SOURCE_NBZ}" 0 "${REPLACEMENT}" "${OUTPUT_NBZ}"
    RESULT_VARIABLE REPEAT_RESULT
    OUTPUT_QUIET
    ERROR_QUIET)
if(REPEAT_RESULT EQUAL 0)
    message(FATAL_ERROR "repeat NBZ copy authoring unexpectedly overwrote output")
endif()
file(SHA256 "${OUTPUT_NBZ}" OUTPUT_SHA_AFTER)
if(NOT OUTPUT_SHA_AFTER STREQUAL OUTPUT_SHA_BEFORE)
    message(FATAL_ERROR "failed repeat NBZ copy authoring changed existing output")
endif()

# Re-enter the canonical DMC3 resolver/acquisition path with the authored copy.
# This proves the writer output is not merely ZIP-readable but consumable by the
# same GDSpaces runtime-volume path used for retail acquisition.
file(COPY_FILE "${OUTPUT_NBZ}" "${VERIFY_DATA}/DMC3-0.nbz" ONLY_IF_DIFFERENT)
set(EXTRACTED "${EXTRACT_OUT}/copy-test.pac")
execute_process(
    COMMAND "${DMC_RENGINE_CLI}" extract-dmc3-retail-member
            "${VERIFY_GAME}" "${GAME_REQUEST}" "${EXTRACTED}"
    RESULT_VARIABLE EXTRACT_RESULT
    OUTPUT_VARIABLE EXTRACT_STDOUT
    ERROR_VARIABLE EXTRACT_STDERR)
if(NOT EXTRACT_RESULT EQUAL 0)
    message(FATAL_ERROR
        "authored NBZ copy failed canonical acquisition (${EXTRACT_RESULT})\nstdout:\n${EXTRACT_STDOUT}\nstderr:\n${EXTRACT_STDERR}")
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${EXTRACTED}" "${REPLACEMENT}"
    RESULT_VARIABLE COMPARE_RESULT)
if(NOT COMPARE_RESULT EQUAL 0)
    message(FATAL_ERROR "authored NBZ copy did not materialize the exact replacement bytes")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
