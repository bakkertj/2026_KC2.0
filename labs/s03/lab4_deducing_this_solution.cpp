// Lab 3.4 solution: deducing this (explicit object parameters)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/15Maa46WP
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

// TODO 1 done: one member; Self is Buffer&, const Buffer&, or Buffer&& as the call site dictates.
class Buffer {
public:
    explicit Buffer(std::vector<int> v) : items_(std::move(v)) {}
    template <class Self>
    auto&& at(this Self&& self, std::size_t i) { return std::forward<Self>(self).items_[i]; }
private:
    std::vector<int> items_;
};

// TODO 2 done: the lambda names itself through `self`; no std::function, fully inlinable.
auto fib = [](this auto&& self, int n) -> int { return n < 2 ? n : self(n - 1) + self(n - 2); };

// TODO 3 done: the setters return whatever type they were called on, so chaining from a
// FrameBuilder stays a FrameBuilder. No CRTP, no static_cast, no template parameter on Builder.
class Builder {
public:
    template <class Self> Self&& id(this Self&& self, int v)              { self.id_ = v; return std::forward<Self>(self); }
    template <class Self> Self&& payload(this Self&& self, std::string s) { self.payload_ = std::move(s); return std::forward<Self>(self); }
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
    std::printf("%s\n", FrameBuilder{}.id(8).payload("pong").build().c_str());
}
