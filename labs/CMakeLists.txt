# The in-class labs: one ten-minute file per lecture block, with a finished version
# beside it. Both must build with the course warning set; the solution's output is
# checked against <name>.expected (regenerate with tools/update-lab-expected.sh).
#
# add_lab(<session> <name> [STARTER_WARNINGS_ONLY])
#   labs/<session>/<name>.cpp           -> lab_<session>_<name>            (build only)
#   labs/<session>/<name>_solution.cpp  -> lab_<session>_<name>_solution   (built and tested)
# STARTER_WARNINGS_ONLY drops -Werror for the starter: the deprecations lab is supposed
# to warn until the attendee fixes it.
function(add_lab session name)
  set(starter lab_${session}_${name})
  add_executable(${starter} ${session}/${name}.cpp)
  if("STARTER_WARNINGS_ONLY" IN_LIST ARGN)
    # The same warning set as course_warnings, without -Werror.
    target_compile_options(${starter} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion
      -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast)
  else()
    target_link_libraries(${starter} PRIVATE course_warnings)
  endif()

  set(solution ${starter}_solution)
  add_executable(${solution} ${session}/${name}_solution.cpp)
  target_link_libraries(${solution} PRIVATE course_warnings)
  add_test(NAME ${solution}
    COMMAND ${CMAKE_COMMAND}
      -DPROGRAM=$<TARGET_FILE:${solution}>
      -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/${session}/${name}.expected
      -P ${CMAKE_SOURCE_DIR}/cmake/expect_output.cmake)
endfunction()

add_lab(s02 lab1_views STARTER_WARNINGS_ONLY)   # Clang flags the dangling return (TODO 3)
add_lab(s02 lab2_optional_variant)
add_lab(s02 lab3_expected_format)
add_lab(s02 lab4_interface_checklist)

add_lab(s03 lab1_constexpr)
add_lab(s03 lab2_templates17)
add_lab(s03 lab3_concepts)
add_lab(s03 lab4_deducing_this)

add_lab(s04 lab1_constrained_algorithms)
add_lab(s04 lab2_views)
add_lab(s04 lab3_cpp23_ranges)
add_lab(s04 lab4_three_ways)

add_lab(s05 lab1_jthread_stop_token)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND COURSE_LIBCXX)
  message(STATUS "labs: lab2_generator skipped (libc++ 18 has no <generator>)")
else()
  add_lab(s05 lab2_generator)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND COURSE_LIBCXX)
  # libc++ removed auto_ptr, random_shuffle and bind2nd outright, so the starter cannot
  # compile there; that is the lab's closing point. Built with GCC/libstdc++ only.
  add_executable(lab_s05_lab3_deprecations_solution s05/lab3_deprecations_solution.cpp)
  target_link_libraries(lab_s05_lab3_deprecations_solution PRIVATE course_warnings)
else()
  add_lab(s05 lab3_deprecations STARTER_WARNINGS_ONLY)
endif()
