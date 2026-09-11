# Demos for session 5 are added here as they are written.
option(COURSE_MODULES "Build the named-modules demo (needs Ninja and CMake 3.28+)" OFF)
if(COURSE_MODULES)
  add_subdirectory(modules)
endif()
