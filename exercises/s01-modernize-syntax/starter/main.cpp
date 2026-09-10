// telemetry: read a record file, compute per-sensor statistics, print a report.
//   usage: telemetry <file.csv>      (or - for stdin)
#include <cstdio>
#include <fstream>
#include <iostream>

#include "telemetry/parser.h"
#include "telemetry/report.h"
#include "telemetry/stats.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <file.csv | ->\n", argv[0]);
        return 2;
    }

    telemetry::LoadResult loaded;
    const std::string path = argv[1];
    if (path == "-") {
        loaded = telemetry::load_stream(std::cin);
    } else {
        std::ifstream in(path.c_str());
        if (!in) {
            std::fprintf(stderr, "cannot open %s\n", path.c_str());
            return 1;
        }
        loaded = telemetry::load_stream(in);
    }

    const telemetry::StatsBySensor stats = telemetry::compute_stats(loaded.records);
    telemetry::write_report(stdout, loaded, stats);
    return loaded.records.empty() ? 1 : 0;
}
