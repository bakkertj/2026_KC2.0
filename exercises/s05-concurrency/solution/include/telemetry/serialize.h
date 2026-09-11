#pragma once
#include <concepts>
#include <ranges>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

#include "telemetry/record.h"

namespace telemetry {

// Session 3: the seven-function serialize overload set becomes constrained
// templates. Each overload says, in its signature, what it accepts; the
// compiler picks the most specific one (concept subsumption), and an argument
// nothing accepts produces an error that names the unmet requirement instead
// of a page of candidates.

// A concept: a named, reusable requirement on a type.
template <typename T>
concept StringLike = std::convertible_to<T, std::string_view>;

// Session 4: the standard range concept replaces the hand-written begin()/end()
// check. It also accepts views, so serialize(v | views::take(2)) works.
template <typename T>
concept SerializableRange = !StringLike<T> && std::ranges::input_range<T>;

// Abbreviated function templates (C++20): `std::integral auto` is a template
// parameter with a constraint. No `template <...>` line needed.
[[nodiscard]] std::string serialize(std::integral auto v) { return std::format("{}", v); }
[[nodiscard]] std::string serialize(std::floating_point auto v) { return std::format("{:.3f}", v); }

// A requires-clause spells the constraint out when the concept has arguments.
template <typename T>
    requires StringLike<T>
[[nodiscard]] std::string serialize(const T& v) {
    return std::format("\"{}\"", std::string_view{v});
}

// Types with a std::formatter but no quoting: enums and Record.
template <typename T>
    requires std::same_as<T, Status> || std::same_as<T, Record>
[[nodiscard]] std::string serialize(const T& v) {
    return std::format("{}", v);
}

// Taken by forwarding reference, not const&: some views (filter_view, drop_while)
// cache state in begin() and cannot be iterated through a const reference.
// This is how the standard algorithms take ranges too.
template <typename R>
    requires SerializableRange<std::remove_cvref_t<R>>
[[nodiscard]] std::string serialize(R&& items) {
    std::string out = "[";
    std::string_view sep;
    for (auto&& item : items) {
        out += sep;
        out += serialize(item);
        sep = ", ";
    }
    out += "]";
    return out;
}

// A fold expression (C++17) over a parameter pack: serialize any number of
// arguments of any serializable types into one bracketed list.
template <typename... Ts>
[[nodiscard]] std::string serialize_all(const Ts&... vs) {
    std::string out = "[";
    std::string_view sep;
    ((out += sep, out += serialize(vs), sep = ", "), ...);
    out += "]";
    return out;
}

// A concept that names "can be serialized", usable in static_asserts and as a
// constraint elsewhere: true iff some serialize overload accepts a T.
template <typename T>
concept Serializable = requires(const T& t) {
    { serialize(t) } -> std::same_as<std::string>;
};

}  // namespace telemetry
