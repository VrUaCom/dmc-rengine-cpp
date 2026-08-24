if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(EXE_DIR "${TEST_ROOT}/game")
set(DATA_DIR "${EXE_DIR}/data/dmc3")
file(MAKE_DIRECTORY "${DATA_DIR}")
file(WRITE "${EXE_DIR}/dmc3.exe" "NOT-A-REAL-DMC3-EXECUTABLE")
file(WRITE "${DATA_DIR}/DMC3-0.nbz" "archive-marker")

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" preflight-dmc3-game-test "${EXE_DIR}"
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE STDERR)

if(RESULT EQUAL 0)
    message(FATAL_ERROR "unknown dmc3.exe must not pass original-game preflight")
endif()
string(FIND "${STDOUT}" "Authority match: unknown" MATCH_POSITION)
if(MATCH_POSITION EQUAL -1)
    message(FATAL_ERROR
        "unknown authority classification missing\nstdout:\n${STDOUT}\nstderr:\n${STDERR}")
endif()
string(FIND "${STDOUT}" "Game-test preflight: READY" READY_POSITION)
if(NOT READY_POSITION EQUAL -1)
    message(FATAL_ERROR "unknown executable incorrectly reported READY")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
