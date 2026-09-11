// telemetry: read a record file, compute per-sensor statistics, print a report.
//   usage: telemetry <file.csv>      (or - for stdin)
//
// Session 5: parsing and statistics run on two threads (see pipeline.h).
#include <cstdio>
#include <fstream>
#include <iostream>
#include <print>
#include <string_view>

#include "telemetry/pipeline.h"
#include "telemetry/report.h"

namespace {

[[nodiscard]] telemetry::PipelineResult run(std::string_view path) {
    if (path == "-") {
        return telemetry::load_and_compute(std::cin);
    }
    std::ifstream in{std::string(path)};
    if (!in) {
        std::println(stderr, "cannot open {}", path);
        return {};
    }
    return telemetry::load_and_compute(in);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::println(stderr, "usage: {} <file.csv | ->", argv[0]);
        return 2;
    }

    const auto [loaded, stats] = run(argv[1]);
    telemetry::write_report(stdout, loaded, stats);
    return loaded.records.empty() ? 1 : 0;
}
