if(NOT DEFINED DMC_RENGINE_CLI)
    message(FATAL_ERROR "DMC_RENGINE_CLI is required")
endif()
if(NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
file(MAKE_DIRECTORY "${TEST_ROOT}")
set(TRACE_PATH "${TEST_ROOT}/synthetic-aborted-l3-trace.json")

file(WRITE "${TRACE_PATH}" [=[
{
  "schema": "dmc-rengine.gdspaces-l3-lifecycle-trace.v1",
  "scope": "V1",
  "status": "aborted",
  "authority": {
    "exe_sha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    "exe_size": 1,
    "role": "synthetic-cli-test-only"
  },
  "resource": {
    "logical_identity": "synthetic/cli-test.pac",
    "selected_provider_identity": "synthetic-provider-only",
    "materialized_sha256": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
    "materialized_size": 0,
    "materialized_provenance": "synthetic-cli-test-only"
  },
  "observer": {
    "name": "synthetic-cli-test-observer",
    "version": "1",
    "config_sha256": "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
    "dropped_events": 0,
    "overflow_detected": false,
    "semantic_intrusion_detected": false
  },
  "run": {
    "id": "synthetic-cli-test-run",
    "original_process": false,
    "completed_cleanly": false,
    "overlay_published": false,
    "rollback_verified": false
  },
  "family_tags": ["synthetic-test-only"],
  "events": [
    {"sequence": 1, "kind": "resource_request"}
  ]
}
]=])

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" validate-l3-lifecycle "${TRACE_PATH}"
    RESULT_VARIABLE VALIDATE_RESULT
    OUTPUT_VARIABLE VALIDATE_OUT
    ERROR_VARIABLE VALIDATE_ERR)
if(NOT VALIDATE_RESULT EQUAL 0)
    message(FATAL_ERROR
        "validate-l3-lifecycle rejected the synthetic diagnostic fixture:\n${VALIDATE_OUT}\n${VALIDATE_ERR}")
endif()
if(NOT VALIDATE_OUT MATCHES "Layer-3 lifecycle trace: valid")
    message(FATAL_ERROR "validation output did not report a valid trace: ${VALIDATE_OUT}")
endif()
if(NOT VALIDATE_OUT MATCHES "Promotion eligible: no")
    message(FATAL_ERROR "manual fixture unexpectedly appeared promotable: ${VALIDATE_OUT}")
endif()

execute_process(
    COMMAND "${DMC_RENGINE_CLI}" validate-l3-lifecycle "${TRACE_PATH}" --require-promotable
    RESULT_VARIABLE PROMOTION_RESULT
    OUTPUT_VARIABLE PROMOTION_OUT
    ERROR_VARIABLE PROMOTION_ERR)
if(PROMOTION_RESULT EQUAL 0)
    message(FATAL_ERROR
        "manual schema-v1 fixture bypassed trusted-origin promotion gate:\n${PROMOTION_OUT}\n${PROMOTION_ERR}")
endif()
if(NOT PROMOTION_ERR MATCHES "trusted instrumentation/publisher binding is required")
    message(FATAL_ERROR
        "promotion failure did not explain the trusted-origin gate:\n${PROMOTION_OUT}\n${PROMOTION_ERR}")
endif()
