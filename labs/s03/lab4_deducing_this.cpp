// Lab 3.4: deducing this (explicit object parameters)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/jorfr6M1v
// Ten minutes, three TODOs in order. Build and run after each one; the output must not
// change. Finished version: lab4_deducing_this_solution.cpp
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

// ---- TODO 1: two overloads that differ only in const -> one template ----------------
// Replace both at() overloads with a single member:
//     template <class Self>
//     auto&& at(this Self&& self, std::size_t i) { return std::forward<Self>(self).items_[i]; }
// The const-ness (and value category) of the object comes in through `self`.
class Buffer {
public:
    explicit Buffer(std::vector<int> v) : items_(std::move(v)) {}
    int&       at(std::size_t i)       { return items_[i]; }
    const int& at(std::size_t i) const { return items_[i]; }
private:
    std::vector<int> items_;
};

// ---- TODO 2: a recursive lambda without std::function ------------------------------
// This lambda can only call itself through a std::function (type-erased, heap-allocating,
// not inlinable). Rewrite it with an explicit object parameter:
//     auto fib = [](this auto&& self, int n) -> int { ... self(n - 1) ... };
std::function<int(int)> fib = [](int n) -> int { return n < 2 ? n : fib(n - 1) + fib(n - 2); };

// ---- TODO 3: a chainable base class without CRTP -----------------------------------
// Builder's setters return Builder&, so `FrameBuilder{}.id(1).payload("x").build()` fails:
// after id(1) the object is a Builder, which has no payload(). The C++11 fix is CRTP
// (template <class Derived> class Builder, with static_casts). Instead, make the setters
// deduce the real type:
//     template <class Self> Self&& id(this Self&& self, int v) { self.id_ = v; return std::forward<Self>(self); }
// Do the same for payload(), then uncomment the chained call in main.
class Builder {
public:
    Builder& id(int v)                  { id_ = v; return *this; }
    Builder& payload(std::string s)     { payload_ = std::move(s); return *this; }
protected:
    int id_ = 0;
    std::string payload_;
};
class FrameBuilder : public Builder {
public:
    std::string build() const { return "frame " + std::to_string(id_) + " [" + payload_ + "]"; }
};

int main() {
    Buffer b({10, 20, 30});
    const Buffer& cb = b;
    b.at(1) = 25;
    std::printf("at: %d %d\n", b.at(1), cb.at(2));

    std::printf("fib(10) = %d\n", fib(10));

    FrameBuilder fb;
    fb.id(7);
    fb.payload("ping");
    std::printf("%s\n", fb.build().c_str());
    // std::printf("%s\n", FrameBuilder{}.id(8).payload("pong").build().c_str());   // TODO 3
}
