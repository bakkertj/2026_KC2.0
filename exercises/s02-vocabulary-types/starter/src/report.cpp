#include "telemetry/report.h"

#include <cmath>

#include "telemetry/config.h"
#include "telemetry/crc.h"
#include "telemetry/serialize.h"

namespace telemetry {

namespace {

[[nodiscard]] const char* severity(Status s) {
    using enum Status;
    switch (s) {
    case Fault:
        return "OUTAGE";
    case Suspect:
        return "flagged";
    case Ok:
        return "";
    }
    return "";
}

[[nodiscard]] unsigned long ul(std::size_t n) { return static_cast<unsigned long>(n); }

}  // namespace

void write_report(std::FILE* out, const LoadResult& loaded, const StatsBySensor& stats) {
    std::fprintf(out, "Telemetry report\n");
    std::fprintf(out, "  lines read: %lu   accepted: %lu   rejected: %lu\n", ul(loaded.lines_read),
                 ul(loaded.records.size()), ul(loaded.lines_read - loaded.records.size()));

    if (!loaded.rejected.empty()) {
        std::fprintf(out, "  rejections by reason:\n");
        for (const auto& [reason, count] : loaded.rejected) {
            std::fprintf(out, "    %-16s %lu\n", to_string(reason), ul(count));
        }
    }

    if (loaded.records.empty()) {
        std::fprintf(out, "  no accepted records\n");
        return;
    }

    const auto [lo, hi] = value_range(loaded.records);
    std::fprintf(out, "  value range: %.3f .. %.3f\n", lo, hi);

    for (const auto& [name, s] : stats) {
        const char* units = "";
        if (const SensorConfig* cfg = find_sensor(name); cfg != nullptr) {
            units = cfg->units;
        }
        std::fprintf(out, "\n%s (%s)\n", name.c_str(), units);
        std::fprintf(out, "  n=%lu  min=%.3f  max=%.3f  mean=%.3f  stddev=%.3f  faults=%lu\n",
                     ul(s.count), s.min, s.max, s.mean, std::sqrt(s.variance()), ul(s.faults));

        std::size_t rank = 0;
        for (const auto& record : top_n_by_value(loaded.records, name, kTopReadings)) {
            const auto line = serialize(record);
            std::fprintf(out, "  #%lu %-48s crc=%04X %s\n", ul(++rank), line.c_str(), crc16(line),
                         severity(record.status));
        }
    }
}

}  // namespace telemetry
