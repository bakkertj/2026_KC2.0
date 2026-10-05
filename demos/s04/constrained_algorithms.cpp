// Demo: std::ranges:: algorithms vs std:: algorithms (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/YjWrjYE6c
#include <algorithm>
#include <print>
#include <string>
#include <vector>

struct Record { long long ts; std::string sensor; double value; };

// [snippet: before]
// C++11: iterator pairs and a comparator lambda, four times in the starter
void sort_by_value_cpp11(std::vector<Record>& v) {
    std::sort(v.begin(), v.end(),
              [](const Record& a, const Record& b) { return a.value < b.value; });
}
// [/snippet]

// [snippet: after]
// C++20: the range, and a projection instead of a comparator
void sort_by_value(std::vector<Record>& v) {
    std::ranges::sort(v, {}, &Record::value);          // {} = std::ranges::less
}
// [/snippet]

// [snippet: why]
// Why std::ranges::sort exists beside std::sort:
//   - takes a range (or an iterator + sentinel), so begin/end mismatches cannot happen
//   - is constrained: sort(list) fails with "does not satisfy random_access_range", not 200 lines
//   - takes a projection, so "compare by member" needs no lambda
//   - returns more: sort returns the end iterator, copy returns both ends, minmax returns the elements
//   - is a function object, not a function: no ADL surprises, cannot be found by unqualified lookup
// [/snippet]

int main() {
    std::vector<Record> v{{1, "a", 3.0}, {2, "b", 1.0}, {3, "c", 2.0}};
    sort_by_value(v);
    std::println("{} {} {}", v[0].value, v[1].value, v[2].value);
}
