# Runs two executables on the same input and fails if their stdout differs.
execute_process(COMMAND ${STARTER} ${DATA} OUTPUT_VARIABLE a RESULT_VARIABLE ra)
execute_process(COMMAND ${SOLUTION} ${DATA} OUTPUT_VARIABLE b RESULT_VARIABLE rb)
if(NOT a STREQUAL b)
  message(FATAL_ERROR "starter and solution reports differ:\n--- starter ---\n${a}\n--- solution ---\n${b}")
endif()
if(NOT ra EQUAL rb)
  message(FATAL_ERROR "exit codes differ: ${ra} vs ${rb}")
endif()
