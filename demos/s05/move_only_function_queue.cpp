// Demo: a task queue with std::move_only_function (C++23)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/jb1GroY51
#include <functional>
#include <memory>
#include <print>
#include <queue>
#include <utility>
#include <version>

int main() {
#ifdef __cpp_lib_move_only_function                  // libstdc++ 12+; libc++ 19+
    // [snippet: tasks]
    std::queue<std::move_only_function<void()>> tasks;    // std::function would refuse the unique_ptr capture
    auto payload = std::make_unique<int>(42);
    tasks.push([p = std::move(payload)] { std::println("task with {}", *p); });
    tasks.push([] { std::println("plain task"); });
    while (!tasks.empty()) { auto t = std::move(tasks.front()); tasks.pop(); t(); }
    // [/snippet]
#else
    std::println("move_only_function not available in this standard library");
#endif
}
