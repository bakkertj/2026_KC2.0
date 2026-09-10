# Toolchain notes (see handouts/toolchain-support-matrix.md):
#   GCC 14 + libstdc++       : everything this course uses.
#   Clang 18 + libstdc++ 14  : NO std::expected (libstdc++ gates it on __cpp_concepts >= 202002L,
#                              which Clang reports only from version 19).
#   Clang 18 + libc++ 18     : std::expected and std::print present; from_chars<double> is NOT
#                              (libc++ 20). The Session 1 solution falls back to strtod there.
# So Clang builds default to libc++. Override with -DCOURSE_LIBCXX=OFF.
option(COURSE_LIBCXX "Use libc++ when compiling with Clang" ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND COURSE_LIBCXX)
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++)
endif()

# Shared warning flags. Every demo and exercise target links course_warnings.
add_library(course_warnings INTERFACE)
target_compile_options(course_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Werror
  -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast)

# doctest, vendored single header
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${CMAKE_SOURCE_DIR}/exercises/common)

# add_demo(<session> <name>) : demos/<session>/<name>.cpp -> executable demo_<session>_<name>
function(add_demo session name)
  set(tgt demo_${session}_${name})
  add_executable(${tgt} ${name}.cpp)
  target_link_libraries(${tgt} PRIVATE course_warnings)
  add_test(NAME ${tgt} COMMAND ${tgt})
endfunction()

# add_exercise_variant(<exercise> <variant> <std>) : builds one variant of an exercise.
#   <variant>/src/*.cpp + <variant>/include  -> static library  <exercise>_<variant>_lib
#   <variant>/main.cpp                       -> executable      <exercise>_<variant>
#   tests/*.cpp                              -> test executable <exercise>_<variant>_tests
# <std> is the C++ standard the variant is compiled as (11 for a C++11 starter, 23 otherwise).
function(add_exercise_variant exercise variant std)
  set(base ${exercise}_${variant})
  file(GLOB srcs CONFIGURE_DEPENDS ${variant}/src/*.cpp)
  # Tests live in <variant>/tests/ when the exercise changes interfaces (Session 2 on),
  # otherwise in a shared tests/ directory.
  file(GLOB tests CONFIGURE_DEPENDS ${variant}/tests/*.cpp)
  if(NOT tests)
    file(GLOB tests CONFIGURE_DEPENDS tests/*.cpp)
  endif()

  add_library(${base}_lib STATIC ${srcs})
  target_include_directories(${base}_lib PUBLIC ${variant}/include)
  target_link_libraries(${base}_lib PUBLIC course_warnings)
  set_target_properties(${base}_lib PROPERTIES CXX_STANDARD ${std})

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${variant}/main.cpp)
    add_executable(${base} ${variant}/main.cpp)
    target_link_libraries(${base} PRIVATE ${base}_lib)
    set_target_properties(${base} PROPERTIES CXX_STANDARD ${std})
  endif()

  add_executable(${base}_tests ${tests})
  target_link_libraries(${base}_tests PRIVATE ${base}_lib doctest)
  set_target_properties(${base}_tests PROPERTIES CXX_STANDARD ${std})
  add_test(NAME ${base}_tests COMMAND ${base}_tests)
endfunction()

# add_report_diff_test(<exercise>) : the starter and solution executables must print
# byte-identical reports for data/sample.csv. The behavior-preservation proof when an
# exercise changes interfaces and therefore its tests.
function(add_report_diff_test exercise)
  add_test(NAME ${exercise}_report_identical
    COMMAND ${CMAKE_COMMAND}
      -DSTARTER=$<TARGET_FILE:${exercise}_starter>
      -DSOLUTION=$<TARGET_FILE:${exercise}_solution>
      -DDATA=${CMAKE_CURRENT_SOURCE_DIR}/data/sample.csv
      -P ${CMAKE_SOURCE_DIR}/cmake/report_diff.cmake)
endfunction()
