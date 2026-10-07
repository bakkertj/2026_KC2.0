// Demo: shared_timed_mutex (C++14), shared_mutex (C++17): reader/writer locks
// Session: s05
// Compiler Explorer: https://godbolt.org/z/sjvr9de5q
#include <map>
#include <mutex>
#include <print>
#include <shared_mutex>
#include <string>
#include <thread>

// [snippet: rwlock]
class SensorTable {
public:
    double limit(const std::string& name) const {
        std::shared_lock lock(mutex_);              // many readers at once
        auto it = limits_.find(name);
        return it == limits_.end() ? 0.0 : it->second;
    }
    void set(const std::string& name, double v) {
        std::unique_lock lock(mutex_);              // one writer, no readers
        limits_[name] = v;
    }
private:
    mutable std::shared_mutex mutex_;               // C++17; C++14: shared_timed_mutex
    std::map<std::string, double> limits_;
};
// Read-mostly data: readers no longer serialize on each other.
// [/snippet]

int main() {
    SensorTable t;
    t.set("rpm", 12000);
    std::jthread r1([&] { std::println("{}", t.limit("rpm")); });
    std::jthread r2([&] { std::println("{}", t.limit("rpm")); });
}
