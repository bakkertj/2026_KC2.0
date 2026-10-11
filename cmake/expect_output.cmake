# Runs an executable and fails if its stdout differs from the contents of EXPECTED.
execute_process(COMMAND ${PROGRAM} OUTPUT_VARIABLE actual RESULT_VARIABLE rc TIMEOUT 30)
file(READ ${EXPECTED} expected)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "${PROGRAM} exited with ${rc}")
endif()
if(NOT actual STREQUAL expected)
  message(FATAL_ERROR "output differs from ${EXPECTED}:\n--- expected ---\n${expected}\n--- actual ---\n${actual}")
endif()
