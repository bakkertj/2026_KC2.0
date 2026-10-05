// Demo: std::atomic<std::shared_ptr> (C++20): hot-swappable configuration
// Session: s05
// Compiler Explorer: <add short link>
#include <atomic>
#include <map>
#include <memory>
#include <print>
#include <string>
#include <thread>
#include <utility>

#ifdef __cpp_lib_atomic_shared_ptr                     // libstdc++ 12+; not libc++ 18
// [snippet: rcu]
struct Config { std::map<std::string, double> limits; };

std::atomic<std::shared_ptr<const Config>> g_config;   // readers copy it; writers swap it

double limit(const std::string& name) {
    auto cfg = g_config.load();                        // one atomic load; cfg keeps the Config alive
    auto it = cfg->limits.find(name);
    return it == cfg->limits.end() ? 0.0 : it->second;
}

void reload(std::map<std::string, double> fresh) {
    g_config.store(std::make_shared<const Config>(std::move(fresh)));   // in-flight readers keep the old one
}
// Read-copy-update with no lock on the read path.
// [/snippet]

int main() {
    reload({{"rpm", 12000}});
    std::jthread reader([] { for (int i = 0; i < 1000; ++i) (void)limit("rpm"); });
    reload({{"rpm", 9000}});
    reader.join();
    std::println("{}", limit("rpm"));
}
#else
int main() { std::println("std::atomic<std::shared_ptr> is not available on this standard library (libc++ 18)"); }
#endif
