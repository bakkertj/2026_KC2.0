// Demo: a coroutine as a state machine (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/xh8jKv6WT
// A byte-at-a-time frame decoder: [0xAA][len][payload...][checksum]. The switch version keeps the
// state in an enum and a counter; the coroutine version keeps it in the program counter.
#include <coroutine>
#include <cstdint>
#include <exception>
#include <optional>
#include <print>
#include <utility>
#include <vector>

using Bytes = std::vector<std::uint8_t>;

// [snippet: switch]
// C++11: the state is data; every transition is a line you must not forget
class SwitchDecoder {
public:
    std::optional<Bytes> feed(std::uint8_t b) {
        using enum State;                       // C++20
        switch (state_) {
        case Sync:    { if (b == 0xAA) { state_ = Len; } break; }
        case Len:     { len_ = b; payload_.clear(); sum_ = 0;
                        state_ = len_ ? Payload : Check; break; }
        case Payload: { payload_.push_back(b); sum_ = add(sum_, b);
                        if (payload_.size() == len_) { state_ = Check; } break; }
        case Check:   { state_ = Sync; if (b == sum_) { return payload_; } break; }
        }
        return std::nullopt;
    }
private:
    static std::uint8_t add(std::uint8_t a, std::uint8_t b) { return static_cast<std::uint8_t>(a + b); }
    enum class State { Sync, Len, Payload, Check } state_ = State::Sync;
    std::size_t len_ = 0;
    std::uint8_t sum_ = 0;
    Bytes payload_;
};
// [/snippet]

// A push-style coroutine: the caller feeds bytes in, the coroutine co_awaits the next one and
// co_yields each completed frame. About forty lines of plumbing, written once per project.
class Decoder {
public:
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;
    struct promise_type {
        std::uint8_t in{};                      // the byte the caller just fed
        std::optional<Bytes> out;               // a frame, when one completes
        Decoder get_return_object() { return Decoder{Handle::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(Bytes frame) { out = std::move(frame); return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };
    // co_await next_byte{} suspends until feed() supplies a byte, then evaluates to it.
    struct next_byte {
        promise_type* p{};
        bool await_ready() const noexcept { return false; }
        void await_suspend(Handle h) noexcept { p = &h.promise(); }
        std::uint8_t await_resume() const noexcept { return p->in; }
    };

    explicit Decoder(Handle h) : h_(h) { h_.resume(); }   // run to the first co_await
    Decoder(Decoder&& o) noexcept : h_(std::exchange(o.h_, {})) {}
    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;
    Decoder& operator=(Decoder&&) = delete;
    ~Decoder() { if (h_) h_.destroy(); }

    std::optional<Bytes> feed(std::uint8_t b) {
        auto& p = h_.promise();
        p.in = b;
        h_.resume();                            // runs until the next co_await or co_yield
        if (!p.out) return std::nullopt;
        auto frame = std::move(*p.out);
        p.out.reset();
        h_.resume();                            // past the co_yield, to the next co_await
        return frame;
    }
private:
    Handle h_;
};

// [snippet: coroutine]
// C++20: the state is where you are in the function; it reads like the spec
Decoder decode() {
    using next = Decoder::next_byte;
    while (true) {
        while (co_await next{} != 0xAA) {}      // Sync
        const std::uint8_t len = co_await next{};
        Bytes payload;
        std::uint8_t sum = 0;
        for (std::uint8_t i = 0; i < len; ++i) {
            const auto b = co_await next{};
            payload.push_back(b);
            sum = static_cast<std::uint8_t>(sum + b);
        }
        if (co_await next{} == sum) co_yield payload;   // Check: hand the frame to the caller
    }
}
// [/snippet]

int main() {
    const Bytes wire{0x00, 0xAA, 0x02, 0x10, 0x20, 0x30, 0xAA, 0x00, 0x00, 0xAA, 0x01, 0x05, 0x06};
    SwitchDecoder s;
    Decoder c = decode();
    int frames_s = 0, frames_c = 0;
    for (auto b : wire) {
        if (auto f = s.feed(b)) { ++frames_s; std::println("switch:    frame of {} bytes", f->size()); }
        if (auto f = c.feed(b)) { ++frames_c; std::println("coroutine: frame of {} bytes", f->size()); }
    }
    std::println("{} frames each (the third has a bad checksum)", frames_s == frames_c ? frames_s : -1);
}
