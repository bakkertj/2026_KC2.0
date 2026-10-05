// Demo: std::filesystem (C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/7YhYWW6Y6
#include <filesystem>
#include <fstream>
#include <print>

namespace fs = std::filesystem;

int main() {
    // [snippet: fs]
    fs::path p = fs::temp_directory_path() / "telemetry" / "run1.csv";   // operator/ joins portably
    std::println("{} {} {}", p.filename().string(), p.extension().string(), p.parent_path().string());

    fs::create_directories(p.parent_path());
    std::ofstream{p} << "ts,sensor,value\n";                 // make the file so the queries have something to find
    std::println("exists: {}", fs::exists(p));            // no exception: a bool
    for (const auto& entry : fs::directory_iterator(p.parent_path())) {
        std::println("{} {}", entry.path().string(), entry.is_regular_file() ? entry.file_size() : 0);
    }
    std::error_code ec;
    fs::remove_all(p.parent_path(), ec);                   // error_code overload: never throws
    // [/snippet]
}
