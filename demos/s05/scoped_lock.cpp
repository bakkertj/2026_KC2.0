// Demo: std::scoped_lock (C++17)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/h3rsPqY6c
#include <mutex>
#include <print>

struct Account { std::mutex m; double balance = 100; };

// [snippet: before]
// C++11: locking two mutexes safely needed std::lock plus two adopt_lock guards
void transfer11(Account& from, Account& to, double amount) {
    std::lock(from.m, to.m);                                      // deadlock-free ordering
    std::lock_guard<std::mutex> a(from.m, std::adopt_lock);
    std::lock_guard<std::mutex> b(to.m, std::adopt_lock);
    from.balance -= amount; to.balance += amount;
}
// [/snippet]

// [snippet: after]
// C++17: one line, same deadlock-avoidance algorithm, CTAD deduces the mutex types
void transfer(Account& from, Account& to, double amount) {
    std::scoped_lock lock(from.m, to.m);
    from.balance -= amount; to.balance += amount;
}
// [/snippet]

int main() {
    Account a, b;
    transfer11(a, b, 10); transfer(b, a, 5);
    std::println("{} {}", a.balance, b.balance);
}
