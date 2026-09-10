#pragma once
#include <cstdio>

#include "telemetry/parser.h"
#include "telemetry/stats.h"

namespace telemetry {

// Number of top readings listed per sensor in the report.
extern const std::size_t kTopReadings;

// Writes the human-readable report. Session 2 replaces printf with
// std::print; Session 4 rewrites the per-sensor section as a ranges pipeline.
void write_report(std::FILE* out, const LoadResult& loaded, const StatsBySensor& stats);

}  // namespace telemetry
