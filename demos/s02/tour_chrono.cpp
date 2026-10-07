// Demo: <chrono> calendars and time zones (C++20)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/rGfKhTe5j
#include <chrono>
#include <print>

int main() {
    using namespace std::chrono;
    // [snippet: chrono]
    // The exercise's `long long` timestamp, with a type:
    sys_time<milliseconds> ts{milliseconds{1725000001000}};
    std::println("{:%F %T} UTC", ts);                         // 2024-08-30 06:40:01.000 UTC

    auto day = floor<days>(ts);                                // truncate to the day
    year_month_day ymd{day};
    std::println("{} {} {}", ymd.year(), ymd.month(), ymd.day());
    std::println("{}", weekday{day});                          // Fri

    auto next = ymd + months{1};                               // calendar arithmetic
    std::println("{}", next);                                  // 2024-09-30

    // Time zones (needs the tzdata database; libstdc++ 13+ ships one)
    // zoned_time local{"America/Chicago", ts};
    // std::println("{:%F %T %Z}", local);
    // [/snippet]
}
