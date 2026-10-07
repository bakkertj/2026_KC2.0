// Demo: deducing this (C++23)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/G8baW1GMh
#include <print>
#include <string>
#include <utility>
#include <vector>

// [snippet: const_dedup]
struct Config {
    std::vector<std::string> names;

    // C++11: two copies of every accessor
    //   const std::string& at(std::size_t i) const { return names[i]; }
    //         std::string& at(std::size_t i)       { return names[i]; }

    // C++23: one. Self deduces as Config&, const Config&, or Config&&, and the return follows.
    template <typename Self>
    auto&& at(this Self&& self, std::size_t i) { return std::forward_like<Self>(self.names[i]); }
};
// [/snippet]

// [snippet: crtp]
// C++11 CRTP: the base needs the derived type as a template parameter.
//   template <typename Derived> struct Shape11 {
//       void draw() { static_cast<Derived*>(this)->draw_impl(); } };
//   struct Circle : Shape11<Circle> { void draw_impl() {...} };

// C++23: the base is a plain struct; self deduces to the derived type at the call site.
struct Shape {
    void draw(this auto&& self) { self.draw_impl(); }
};
struct Circle : Shape { void draw_impl() { std::println("circle"); } };
struct Square : Shape { void draw_impl() { std::println("square"); } };
// [/snippet]

// [snippet: recursive]
// A recursive lambda without std::function or a Y-combinator:
auto fib = [](this auto self, int n) -> int { return n < 2 ? n : self(n - 1) + self(n - 2); };
// [/snippet]

// [snippet: chain]
struct Builder {
    std::string out;
    template <typename Self>
    Self&& add(this Self&& self, std::string_view s) { self.out += s; return std::forward<Self>(self); }
};
// Builder{}.add("a").add("b") moves the temporary through each call; Builder b; b.add("a") returns b&.
// [/snippet]

int main() {
    Config c{{"rpm", "temp"}};
    const Config& cc = c;
    c.at(0) = "RPM";                                       // Config& -> std::string&
    std::println("{} {}", cc.at(0), Config{{"x"}}.at(0));  // const& -> const&; && -> &&
    Circle{}.draw(); Square{}.draw();
    std::println("{} {}", fib(10), Builder{}.add("a").add("b").out);
}
