// Demo: using enum (C++20)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstdio>

enum class Status { Ok, Suspect, Fault };

// [snippet: before]
const char* to_string_cpp11(Status s) {
    switch (s) {
    case Status::Ok:      return "ok";
    case Status::Suspect: return "suspect";
    case Status::Fault:   return "fault";
    }
    return "?";
}
// [/snippet]

// [snippet: after]
const char* to_string(Status s) {
    using enum Status;          // in scope for this block
    switch (s) {
    case Ok:      return "ok";
    case Suspect: return "suspect";
    case Fault:   return "fault";
    }
    return "?";
}
// [/snippet]

int main() { std::printf("%s %s\n", to_string_cpp11(Status::Ok), to_string(Status::Fault)); }
