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
  file(GLOB tests CONFIGURE_DEPENDS tests/*.cpp)

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
