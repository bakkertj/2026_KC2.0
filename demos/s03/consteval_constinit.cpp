// Demo: consteval and constinit (C++20)
// Session: s03
// Compiler Explorer: <add short link>
#include <cstdint>
#include <print>
#include <string_view>

// [snippet: consteval]
// consteval: an immediate function. It CANNOT be called at runtime.
consteval std::uint32_t fnv1a(std::string_view s) {
    std::uint32_t h = 2166136261u;
    for (char c : s) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    return h;
}

constexpr auto kRpmId = fnv1a("rpm");          // fine: a constant
// std::uint32_t id(std::string_view s) { return fnv1a(s); }   // error: s is not a constant
// A hash that is guaranteed never to run at startup or in a hot loop, and cannot be
// misused to do so.
// [/snippet]

// [snippet: validate]
// consteval with a message: return why it failed, and let static_assert print it.
struct Limit { std::string_view name; double lo, hi; };
consteval std::string_view validate(const Limit& l) {
    if (l.name.empty()) return "limit has no name";
    if (!(l.lo < l.hi)) return "limit has lo >= hi";
    return {};
}
constexpr Limit kRpm{"rpm", 0.0, 12000.0};
static_assert(validate(kRpm).empty(), "sensor limit is invalid");
// [/snippet]

// [snippet: constinit]
// constinit: initialized at compile time, mutable at runtime. No static-init-order fiasco.
struct Counters { std::uint32_t parsed = 0; std::uint32_t rejected = 0; };
constinit Counters g_counters{};                        // zero-initialized before any code runs, guaranteed

// constinit int bad = fnv1a_runtime("x");             // error if the initializer is not a constant
// A constexpr global would be immutable; a plain global might be dynamically initialized
// after something else already used it. constinit is the third option.
// [/snippet]

int main() { ++g_counters.parsed; std::println("{:08X} {}", kRpmId, g_counters.parsed); }
