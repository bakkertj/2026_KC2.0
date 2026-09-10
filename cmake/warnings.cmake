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

# add_exercise_variant(<exercise> <variant>) : builds starter or solution with its tests
function(add_exercise_variant exercise variant)
  set(tgt ${exercise}_${variant})
  file(GLOB srcs CONFIGURE_DEPENDS ${variant}/*.cpp)
  file(GLOB tests CONFIGURE_DEPENDS tests/*.cpp)
  add_executable(${tgt} ${srcs} ${tests})
  target_include_directories(${tgt} PRIVATE ${variant})
  target_link_libraries(${tgt} PRIVATE course_warnings doctest)
  add_test(NAME ${tgt} COMMAND ${tgt})
endfunction()
