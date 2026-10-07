// Demo: a formatter that accepts its own format spec (C++20)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/jefW7WGYx
#include <format>
#include <print>

struct Celsius { double v; };

// [snippet: spec]
// Supports {} (one decimal) and {:f} (fahrenheit): parse() reads the spec once, format() uses it
template <>
struct std::formatter<Celsius> {
    bool fahrenheit = false;

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'f') { fahrenheit = true; ++it; }
        if (it != ctx.end() && *it != '}') throw std::format_error("Celsius: spec is '' or 'f'");
        return it;
    }
    template <typename Ctx>
    auto format(Celsius c, Ctx& ctx) const {
        return fahrenheit ? std::format_to(ctx.out(), "{:.1f}F", c.v * 9 / 5 + 32)
                          : std::format_to(ctx.out(), "{:.1f}C", c.v);
    }
};
// [/snippet]

int main() { std::println("{} {:f}", Celsius{41.25}, Celsius{41.25}); }
