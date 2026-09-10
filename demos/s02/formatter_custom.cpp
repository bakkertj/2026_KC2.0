// Demo: std::formatter for your own types (C++20)
// Session: s02
// Compiler Explorer: <add short link>
#include <format>
#include <print>
#include <string>
#include <string_view>

enum class Status { Ok, Fault };
constexpr std::string_view to_string(Status s) { return s == Status::Ok ? "ok" : "fault"; }

struct Record { long long ts; std::string sensor; double value; Status status; };

// [snippet: simple]
// An enum-like type: inherit parse() and the width/alignment handling from formatter<string_view>
template <>
struct std::formatter<Status> : std::formatter<std::string_view> {
    template <typename Ctx>
    auto format(Status s, Ctx& ctx) const {
        return std::formatter<std::string_view>::format(to_string(s), ctx);
    }
};
// [/snippet]

// [snippet: full]
// A struct: write parse() (accept an empty spec) and format() (delegate to format_to)
template <>
struct std::formatter<Record> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    template <typename Ctx>
    auto format(const Record& r, Ctx& ctx) const {
        return std::format_to(ctx.out(), "{{{},\"{}\",{:.3f},{}}}", r.ts, r.sensor, r.value, r.status);
    }
};
// format() is a template on the context: the standard allows any basic_format_context,
// and libc++ checks at compile time that yours does.
// [/snippet]

int main() {
    Record r{7, "rpm", 1.5, Status::Ok};
    std::println("{} [{:>8}] {}", r, Status::Fault, std::format("{}", r).size());
}
