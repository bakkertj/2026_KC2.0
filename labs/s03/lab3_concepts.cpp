// Lab 3.3: concepts (requires expressions and constrained overloads)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/h4Kabrb5Y
// Ten minutes, three TODOs in order. Build and run after each one.
// Finished version: lab3_concepts_solution.cpp
#include <concepts>
#include <cstdio>
#include <string>
#include <vector>

struct Reading { int sensor; double value; };
struct Opaque  { int bits; };

std::string serialize(int v)             { return std::to_string(v); }
std::string serialize(double v)          { return std::to_string(v); }
std::string serialize(const Reading& r)  { return std::to_string(r.sensor) + ":" + std::to_string(r.value); }
// note: no serialize(Opaque)

// ---- TODO 1: name the requirement -------------------------------------------------
// Write a concept Serializable: T satisfies it when serialize(t) is valid for a const T&
// and yields something convertible to std::string.
//     template <class T>
//     concept Serializable = requires(const T& t) {
//         { serialize(t) } -> std::convertible_to<std::string>;
//     };
// Then uncomment the static_asserts.
// static_assert(Serializable<int> && Serializable<Reading>);
// static_assert(!Serializable<Opaque>);

// ---- TODO 2: constrain the template -----------------------------------------------
// to_json accepts anything and fails deep inside when the type has no serialize().
// Constrain it with the concept (any of the four spellings from the slides). Then try
// to_json(Opaque{1}) once (uncomment the line in main), read the error message, and
// compare it with the one you got before the constraint. Comment it out again.
template <class T>
std::string to_json(const T& v) { return "{\"v\":" + serialize(v) + "}"; }

// ---- TODO 3: let the more specific overload win -----------------------------------
// We want to_json on a range of Serializable elements to produce a JSON array.
// Add a second overload constrained by a concept SerializableRange:
//     template <class R>
//     concept SerializableRange = std::ranges::range<R> && Serializable<std::ranges::range_value_t<R>>;
// A vector<Reading> is not itself Serializable, so only the new overload is viable and the
// call is unambiguous. (If a type satisfied both, the overload whose constraint subsumes
// the other would win; that only works when the constraints are spelled with named
// concepts, which is why SerializableRange is written the way it is.)
// Expected output for the vector: {"v":[1:2.500000,2:3.000000]}
// (Hint: #include <ranges>; loop with a range-for and join with commas.)

int main() {
    std::printf("%s\n", to_json(42).c_str());
    std::printf("%s\n", to_json(Reading{1, 2.5}).c_str());
    // std::printf("%s\n", to_json(Opaque{1}).c_str());          // TODO 2: read the error, then re-comment
    std::vector<Reading> rs{{1, 2.5}, {2, 3.0}};
    // std::printf("%s\n", to_json(rs).c_str());                 // TODO 3
}
