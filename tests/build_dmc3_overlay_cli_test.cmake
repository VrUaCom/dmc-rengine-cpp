if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(GAME_ROOT "${TEST_ROOT}/game")
set(DATA_DIR "${GAME_ROOT}/data/dmc3")
set(OUTPUT_DIR "${TEST_ROOT}/output")
set(AUTHORED_FILE "${TEST_ROOT}/authored.pac")
set(OUTPUT_FILE "${OUTPUT_DIR}/DMC3-1.nbz")
file(MAKE_DIRECTORY "${DATA_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# Discovery only consumes numbered-volume filenames. The existing volume does
# not need to be opened by this artifact-building command.
file(WRITE "${DATA_DIR}/DMC3-0.nbz" "existing-volume-marker")
file(WRITE "${AUTHORED_FILE}" "AUTHORED-RESOURCE-BYTES")

execute_process(
    COMMAND "${DMC_RENGINE_CLI}"
            build-dmc3-overlay
            "${GAME_ROOT}"
            "obj\\em000.pac"
            "${AUTHORED_FILE}"
            "${OUTPUT_DIR}"
    RESULT_VARIABLE BUILD_RESULT
    OUTPUT_VARIABLE BUILD_STDOUT
    ERROR_VARIABLE BUILD_STDERR)
if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR
        "build-dmc3-overlay failed (${BUILD_RESULT})\nstdout:\n${BUILD_STDOUT}\nstderr:\n${BUILD_STDERR}")
endif()
if(NOT EXISTS "${OUTPUT_FILE}")
    message(FATAL_ERROR "expected next contiguous overlay DMC3-1.nbz")
endif()
if(EXISTS "${DATA_DIR}/DMC3-1.nbz")
    message(FATAL_ERROR "retail game data directory was modified")
endif()
string(FIND "${BUILD_STDOUT}" "Archive member: GDataX360.afs/em000.pac" MEMBER_POSITION)
if(MEMBER_POSITION EQUAL -1)
    message(FATAL_ERROR "runtime archive candidate was not derived from basename policy")
endif()
string(FIND "${BUILD_STDOUT}" "Publication: atomic/no-replace output-only" PUBLICATION_POSITION)
if(PUBLICATION_POSITION EQUAL -1)
    message(FATAL_ERROR "atomic/no-replace publication receipt is missing")
endif()

# A second publication to the same path must fail and leave the first artifact
# byte-identical. This is the CLI boundary for the shared no-replace contract.
file(SHA256 "${OUTPUT_FILE}" FIRST_OUTPUT_SHA)
execute_process(
    COMMAND "${DMC_RENGINE_CLI}"
            build-dmc3-overlay
            "${GAME_ROOT}"
            "obj\\em000.pac"
            "${AUTHORED_FILE}"
            "${OUTPUT_DIR}"
    RESULT_VARIABLE REPEAT_RESULT
    OUTPUT_VARIABLE REPEAT_STDOUT
    ERROR_VARIABLE REPEAT_STDERR)
if(REPEAT_RESULT EQUAL 0)
    message(FATAL_ERROR "second publication to an existing artifact must fail closed")
endif()
file(SHA256 "${OUTPUT_FILE}" SECOND_OUTPUT_SHA)
if(NOT FIRST_OUTPUT_SHA STREQUAL SECOND_OUTPUT_SHA)
    message(FATAL_ERROR "failed second publication modified the existing artifact")
endif()

execute_process(
    COMMAND "${DMC_RENGINE_CLI}"
            build-dmc3-overlay
            "${GAME_ROOT}"
            "obj\\em001.pac"
            "${AUTHORED_FILE}"
            "${DATA_DIR}"
    RESULT_VARIABLE RETAIL_RESULT
    OUTPUT_VARIABLE RETAIL_STDOUT
    ERROR_VARIABLE RETAIL_STDERR)
if(RETAIL_RESULT EQUAL 0)
    message(FATAL_ERROR "publication into retail data directory must fail closed")
endif()
if(EXISTS "${DATA_DIR}/DMC3-1.nbz")
    message(FATAL_ERROR "failed publication attempt modified retail game data")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
