// Demo: variant as a closed message set (C++17)
// Session: s02
// Compiler Explorer: <add short link>
#include <cstdint>
#include <cstdio>
#include <string>
#include <variant>
#include <vector>

// [snippet: messages]
struct Reading   { std::uint16_t sensor; double value; };
struct Heartbeat { std::uint32_t uptime_s; };
struct Fault     { std::uint16_t code; std::string detail; };

using Message = std::variant<Reading, Heartbeat, Fault>;   // the whole protocol, in one line

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };

void handle(const Message& m) {
    std::visit(overloaded{
        [](const Reading& r)   { std::printf("reading %u = %.2f\n", r.sensor, r.value); },
        [](const Heartbeat& h) { std::printf("alive %us\n", h.uptime_s); },
        [](const Fault& f)     { std::printf("FAULT %u: %s\n", f.code, f.detail.c_str()); },
    }, m);
}
// [/snippet]

// [snippet: vs_virtual]
// variant: closed set of types, open set of operations (add a visitor anywhere)
// virtual: open set of types, closed set of operations (add a method to the base)
// Protocols and ASTs are closed sets of types. Plugins are open sets. Choose accordingly.
// [/snippet]

int main() {
    std::vector<Message> log{Reading{3, 41.25}, Heartbeat{120}, Fault{7, "overtemp"}};
    for (const auto& m : log) handle(m);
    std::printf("%zu bytes per message\n", sizeof(Message));
}
