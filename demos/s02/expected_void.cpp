// Demo: expected<void, E> (C++23)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/fzdj6x5rz
#include <cstdio>
#include <expected>
#include <string_view>

enum class IoError { NotOpen, Full };

// [snippet: void]
// "Did it work, and if not, why": no value to return, but a reason to report.
std::expected<void, IoError> write(std::string_view data, bool open) {
    if (!open) return std::unexpected(IoError::NotOpen);
    if (data.size() > 64) return std::unexpected(IoError::Full);
    return {};                                    // success: an empty expected
}

void use() {
    if (auto r = write("hello", true); !r) {
        std::printf("failed: %d\n", static_cast<int>(r.error()));
    }
}
// [/snippet]

int main() { use(); std::printf("%d\n", write("x", false).has_value()); }
