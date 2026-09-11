#pragma once
#include <iosfwd>
#include <stop_token>
#include <version>

#include "telemetry/parser.h"
#include "telemetry/stats.h"

#if __has_include(<generator>) && defined(__cpp_lib_generator)
#include <generator>
#define TELEMETRY_HAS_GENERATOR 1
#endif

namespace telemetry {

// Session 5: the parse step and the statistics step run on different threads.
// A producer (std::jthread) parses lines and pushes Records into a bounded
// queue; the consumer (the calling thread) pops them and accumulates
// statistics. Records still end up in LoadResult, in input order, so the
// report is byte-identical to the single-threaded version.
struct PipelineResult {
    LoadResult loaded;
    StatsBySensor stats;
};

// Runs the two-stage pipeline. A stop request on `stop` ends processing early
// (the partial results are returned).
[[nodiscard]] PipelineResult load_and_compute(std::istream& in, std::stop_token stop = {});

#ifdef TELEMETRY_HAS_GENERATOR
// The parser as a coroutine: a lazy range of accepted Records. Rejected lines
// are skipped (use load_stream for the counts). Each co_yield suspends the
// coroutine until the consumer asks for the next record, so a file is never
// fully in memory and the caller can stop at any point.
//   for (const Record& r : records(file) | std::views::take(10)) ...
[[nodiscard]] std::generator<Record> records(std::istream& in);
#endif

}  // namespace telemetry
