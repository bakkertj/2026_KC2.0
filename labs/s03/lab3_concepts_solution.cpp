// Lab 3.3 solution: concepts (requires expressions and constrained overloads)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/a4xY5sa9f
#include <concepts>
#include <cstdio>
#include <ranges>
#include <string>
#include <vector>

struct Reading { int sensor; double value; };
struct Opaque  { int bits; };

std::string serialize(int v)             { return std::to_string(v); }
std::string serialize(double v)          { return std::to_string(v); }
std::string serialize(const Reading& r)  { return std::to_string(r.sensor) + ":" + std::to_string(r.value); }

// TODO 1 done: the requirement has a name, and the name is checkable.
template <class T>
concept Serializable = requires(const T& t) {
    { serialize(t) } -> std::convertible_to<std::string>;
};
static_assert(Serializable<int> && Serializable<Reading>);
static_assert(!Serializable<Opaque>);

// TODO 2 done: the error for Opaque now says "constraints not satisfied" at the call,
// instead of "no matching function for call to serialize" three levels down.
template <Serializable T>
std::string to_json(const T& v) { return "{\"v\":" + serialize(v) + "}"; }

// TODO 3 done: a range of serializable things becomes a JSON array.
template <class R>
concept SerializableRange = std::ranges::range<R> && Serializable<std::ranges::range_value_t<R>>;

template <SerializableRange R>
std::string to_json(const R& r) {
    std::string out = "{\"v\":[";
    const char* sep = "";
    for (const auto& e : r) { out += sep; out += serialize(e); sep = ","; }
    return out + "]}";
}

int main() {
    std::printf("%s\n", to_json(42).c_str());
    std::printf("%s\n", to_json(Reading{1, 2.5}).c_str());
    // to_json(Opaque{1});   // error: constraints not satisfied: 'Opaque' does not satisfy 'Serializable'
    std::vector<Reading> rs{{1, 2.5}, {2, 3.0}};
    std::printf("%s\n", to_json(rs).c_str());
}
