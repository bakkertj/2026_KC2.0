#include "telemetry/pipeline.h"

#include <istream>
#include <string>
#include <thread>
#include <utility>
#include <variant>

#include "telemetry/queue.h"

namespace telemetry {

namespace {

// What the producer sends: an accepted Record, or the reason a line was rejected.
// One queue, one message type (Session 2: a closed set of alternatives is a variant).
using Message = std::variant<Record, ParseError>;

}  // namespace

PipelineResult load_and_compute(std::istream& in, std::stop_token stop) {
    BoundedQueue<Message> queue;
    std::size_t lines_read = 0;

    // The producer. std::jthread joins in its destructor and can be asked to
    // stop; its stop_token is passed as the first argument automatically.
    std::jthread producer([&](std::stop_token producer_stop) {
        std::string line;
        while (!producer_stop.stop_requested() && std::getline(in, line)) {
            ++lines_read;   // only the producer writes this; read after join()
            if (line.ends_with('\r')) line.pop_back();
            auto parsed = parse_record(line);
            Message m = parsed ? Message{std::move(*parsed)} : Message{parsed.error()};
            if (!queue.push(std::move(m))) break;
        }
        queue.close();
    });

    // A stop request from the caller propagates to the producer.
    std::stop_callback propagate(stop, [&] { producer.request_stop(); queue.close(); });

    // The consumer: this thread.
    PipelineResult result;
    while (auto m = queue.pop(stop)) {
        std::visit(
            [&](auto&& v) {
                using V = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<V, Record>) {
                    result.stats.try_emplace(v.sensor).first->second.add(v.value, v.status);
                    result.loaded.records.push_back(std::move(v));
                } else {
                    ++result.loaded.rejected[v];
                }
            },
            std::move(*m));
    }

    producer.join();   // explicit, so lines_read is safely visible afterward
    result.loaded.lines_read = lines_read;
    return result;
}

#ifdef TELEMETRY_HAS_GENERATOR
std::generator<Record> records(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.ends_with('\r')) line.pop_back();
        if (auto parsed = parse_record(line)) co_yield std::move(*parsed);
    }
}
#endif

}  // namespace telemetry
