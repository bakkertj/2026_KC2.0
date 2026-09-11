#include "telemetry/report.h"

#include <algorithm>
#include <cmath>
#include <print>
#include <ranges>

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

    // Group the accepted records by sensor: sort a copy by sensor name, then
    // chunk_by splits it into one subrange per sensor. Each group is then a
    // pipeline: sort by value descending, take the top N, number them.
    auto by_sensor = loaded.records | std::ranges::to<std::vector>();
    std::ranges::sort(by_sensor, {}, &Record::sensor);

    for (auto group : by_sensor | std::views::chunk_by([](const Record& a, const Record& b) {
                          return a.sensor == b.sensor;
                      })) {
        const std::string& name = group.front().sensor;
        const SensorStats& s = stats.find(name)->second;
        const auto units = find_sensor(name).transform([](const SensorConfig& c) { return c.units; }).value_or("");

        std::println(out, "");
        std::println(out, "{} ({})", name, units);
        std::println(out, "  n={}  min={:.3f}  max={:.3f}  mean={:.3f}  stddev={:.3f}  faults={}", s.count,
                     s.min, s.max, s.mean, std::sqrt(s.variance()), s.faults);

        auto top = group | std::ranges::to<std::vector>();
        std::ranges::stable_sort(top, std::ranges::greater{}, &Record::value);
        // zip pairs each record with its rank. (views::enumerate does the same from 0,
        // but libc++ 18 lacks it; zip with iota is the portable spelling.)
        for (const auto& [rank, record] : std::views::zip(std::views::iota(1uz), top | std::views::take(kTopReadings))) {
            const auto line = std::format("{}", record);
            std::println(out, "  #{} {:<48} crc={:04X} {}", rank, line, crc16(line), severity(record.status));
        }
    }
}

}  // namespace telemetry
