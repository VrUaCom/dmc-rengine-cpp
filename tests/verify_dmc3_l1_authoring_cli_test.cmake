if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(EXE_DIR "${TEST_ROOT}/game")
set(DATA_DIR "${EXE_DIR}/data/dmc3")
set(REPLACEMENT "${TEST_ROOT}/replacement.bin")
set(EXTERNAL_WORKSPACE "${TEST_ROOT}/workspace")
set(IN_TREE_WORKSPACE "${EXE_DIR}/workspace")
file(MAKE_DIRECTORY "${DATA_DIR}")
file(WRITE "${EXE_DIR}/dmc3.exe" "NOT-A-RECOGNIZED-DMC3-EXECUTABLE")
file(WRITE "${DATA_DIR}/DMC3-0.nbz" "RETAIL-ARCHIVE-MUST-REMAIN-UNTOUCHED")
file(WRITE "${REPLACEMENT}" "REPLACEMENT-BYTES")
file(SHA256 "${DATA_DIR}/DMC3-0.nbz" ARCHIVE_SHA_BEFORE)

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" verify-dmc3-l1-authoring
            "${EXE_DIR}" "obj\\em000.pac" "0"
            "${REPLACEMENT}" "${IN_TREE_WORKSPACE}"
    RESULT_VARIABLE IN_TREE_RESULT
    OUTPUT_VARIABLE IN_TREE_STDOUT
    ERROR_VARIABLE IN_TREE_STDERR)
if(IN_TREE_RESULT EQUAL 0)
    message(FATAL_ERROR "workspace inside the protected executable tree unexpectedly succeeded")
endif()
string(FIND "${IN_TREE_STDERR}" "workspace must be outside the complete retail executable tree" IN_TREE_GUARD)
if(IN_TREE_GUARD EQUAL -1)
    message(FATAL_ERROR
        "protected-tree guard did not report the expected failure\nstdout:\n${IN_TREE_STDOUT}\nstderr:\n${IN_TREE_STDERR}")
endif()
if(EXISTS "${IN_TREE_WORKSPACE}")
    message(FATAL_ERROR "protected-tree rejection created a workspace")
endif()

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" verify-dmc3-l1-authoring
            "${EXE_DIR}" "obj\\em000.pac" "0"
            "${REPLACEMENT}" "${EXTERNAL_WORKSPACE}"
    RESULT_VARIABLE UNKNOWN_EXE_RESULT
    OUTPUT_VARIABLE UNKNOWN_EXE_STDOUT
    ERROR_VARIABLE UNKNOWN_EXE_STDERR)
if(UNKNOWN_EXE_RESULT EQUAL 0)
    message(FATAL_ERROR "unknown executable unexpectedly passed L1 authoring preflight")
endif()
string(FIND "${UNKNOWN_EXE_STDERR}" "protected distribution preflight failed" PREFLIGHT_GUARD)
if(PREFLIGHT_GUARD EQUAL -1)
    message(FATAL_ERROR
        "unknown executable did not stop at protected-distribution preflight\nstdout:\n${UNKNOWN_EXE_STDOUT}\nstderr:\n${UNKNOWN_EXE_STDERR}")
endif()
if(EXISTS "${EXTERNAL_WORKSPACE}")
    message(FATAL_ERROR "failed protected-distribution preflight created closure artifacts")
endif()

file(SHA256 "${DATA_DIR}/DMC3-0.nbz" ARCHIVE_SHA_AFTER)
if(NOT ARCHIVE_SHA_AFTER STREQUAL ARCHIVE_SHA_BEFORE)
    message(FATAL_ERROR "fail-closed paths changed the retail archive")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
