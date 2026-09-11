// Demo: the range concept hierarchy (C++20)
// Session: s04
// Compiler Explorer: <add short link>
#include <deque>
#include <forward_list>
#include <list>
#include <print>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

// [snippet: hierarchy]
namespace r = std::ranges;
using V = std::vector<int>;
static_assert(r::contiguous_range<V> && r::contiguous_range<std::span<int>> && r::contiguous_range<std::string_view>);
static_assert(r::random_access_range<std::deque<int>> && !r::contiguous_range<std::deque<int>>);
static_assert(r::bidirectional_range<std::list<int>> && !r::random_access_range<std::list<int>>);
static_assert(r::forward_range<std::forward_list<int>> && !r::bidirectional_range<std::forward_list<int>>);

// Every level includes the ones below it: a vector is also an input_range.
static_assert(r::input_range<V> && r::forward_range<V> && r::bidirectional_range<V>);

// Views are ranges too, at the level their source and adaptor allow.
static_assert(r::random_access_range<decltype(std::views::iota(0, 10))>);
static_assert(r::bidirectional_range<decltype(V{} | std::views::filter([](int) { return true; }))>);
static_assert(!r::random_access_range<decltype(V{} | std::views::filter([](int) { return true; }))>);   // filter loses it

static_assert(r::sized_range<V> && !r::sized_range<std::forward_list<int>>);
static_assert(r::view<std::span<int>> && r::view<std::string_view> && !r::view<V>);   // views: cheap to copy, non-owning
static_assert(r::borrowed_range<std::span<int>> && !r::borrowed_range<V>);            // iterators may outlive the object
// [/snippet]

int main() { std::println("all range concept checks passed at compile time"); }
