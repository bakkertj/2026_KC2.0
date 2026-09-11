# Named modules need: CMake 3.28+, the Ninja (or Visual Studio) generator, and a compiler
# with P1689 dependency scanning: GCC 14+, Clang 16+, MSVC 17.4+. Off by default; enable with
#   cmake -G Ninja -DCOURSE_MODULES=ON ...
# `import std;` additionally needs CMake 3.30+ and libc++ 17+ or libstdc++ 15 (CMAKE_CXX_MODULE_STD).
add_executable(demo_s05_modules main.cpp)
target_sources(demo_s05_modules PUBLIC FILE_SET CXX_MODULES FILES crc.cppm)
target_link_libraries(demo_s05_modules PRIVATE course_warnings)
add_test(NAME demo_s05_modules COMMAND demo_s05_modules)
