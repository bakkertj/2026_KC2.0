#pragma once
#include <cstdio>

#include "telemetry/parser.h"
#include "telemetry/stats.h"

namespace telemetry {

// Number of top readings listed per sensor in the report.
inline constexpr std::size_t kTopReadings = 3;

// Writes the human-readable report with std::print. Still takes a FILE* so the
// tests can capture it with tmpfile(); std::print has a FILE* overload.
// Session 4 rewrites the per-sensor section as a ranges pipeline.
void write_report(std::FILE* out, const LoadResult& loaded, const StatsBySensor& stats);

}  // namespace telemetry
