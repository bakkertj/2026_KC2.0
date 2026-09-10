#include "telemetry/report.h"

#include <cmath>
#include <print>

#include "telemetry/config.h"
#include "telemetry/crc.h"
#include "telemetry/serialize.h"

namespace telemetry {

namespace {

[[nodiscard]] std::string_view severity(Status s) {
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

}  // namespace

void write_report(std::FILE* out, const LoadResult& loaded, const StatsBySensor& stats) {
    std::println(out, "Telemetry report");
    std::println(out, "  lines read: {}   accepted: {}   rejected: {}", loaded.lines_read,
                 loaded.records.size(), loaded.lines_read - loaded.records.size());

    if (!loaded.rejected.empty()) {
        std::println(out, "  rejections by reason:");
        for (const auto& [reason, count] : loaded.rejected) {
            std::println(out, "    {:<16} {}", reason, count);
        }
    }

    const auto range = value_range(loaded.records);
    if (!range) {
        std::println(out, "  no accepted records");
        return;
    }
    std::println(out, "  value range: {:.3f} .. {:.3f}", range->first, range->second);

    for (const auto& [name, s] : stats) {
        // optional::transform (C++23): units if the sensor is known, else "".
        const auto units =
            find_sensor(name).transform([](const SensorConfig& c) { return c.units; }).value_or("");
        std::println(out, "");
        std::println(out, "{} ({})", name, units);
        std::println(out, "  n={}  min={:.3f}  max={:.3f}  mean={:.3f}  stddev={:.3f}  faults={}", s.count,
                     s.min, s.max, s.mean, std::sqrt(s.variance()), s.faults);

        std::size_t rank = 0;
        for (const auto& record : top_n_by_value(loaded.records, name, kTopReadings)) {
            const auto line = std::format("{}", record);
            std::println(out, "  #{} {:<48} crc={:04X} {}", ++rank, line, crc16(line),
                         severity(record.status));
        }
    }
}

}  // namespace telemetry
