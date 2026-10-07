// Demo: std::any (C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/YTM8YdMjf
#include <any>
#include <cstdio>
#include <map>
#include <string>

// [snippet: any]
std::map<std::string, std::any> properties;   // a bag of values whose types are known only to the caller

void use() {
    properties["retries"] = 3;
    properties["name"] = std::string("gimbal");

    int r = std::any_cast<int>(properties["retries"]);            // throws std::bad_any_cast if wrong
    if (auto* s = std::any_cast<std::string>(&properties["name"])) {   // nullptr if wrong
        std::printf("%d %s\n", r, s->c_str());
    }
    // properties["retries"].type() == typeid(int)
}
// [/snippet]

int main() { use(); }
