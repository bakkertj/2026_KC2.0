// telemetry: read a record file, compute per-sensor statistics, print a report.
//   usage: telemetry <file.csv>      (or - for stdin)
#include <cstdio>
#include <fstream>
#include <iostream>
#include <print>
#include <string_view>

#include "telemetry/parser.h"
#include "telemetry/report.h"
#include "telemetry/stats.h"

namespace {

[[nodiscard]] telemetry::LoadResult load(std::string_view path) {
    if (path == "-") {
        return telemetry::load_stream(std::cin);
    }
    std::ifstream in{std::string(path)};
    if (!in) {
        std::println(stderr, "cannot open {}", path);
        return {};
    }
    return telemetry::load_stream(in);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::println(stderr, "usage: {} <file.csv | ->", argv[0]);
        return 2;
    }

    const auto loaded = load(argv[1]);
    const auto stats = telemetry::compute_stats(loaded.records);
    telemetry::write_report(stdout, loaded, stats);
    return loaded.records.empty() ? 1 : 0;
}
