#include "telemetry/report.h"

#include <cmath>

#include "telemetry/config.h"
#include "telemetry/crc.h"
#include "telemetry/serialize.h"

namespace telemetry {

const std::size_t kTopReadings = 3;

namespace {

const char* severity(Status s) {
    // Fault and Suspect both flag the sensor; only Fault is counted as an outage.
    switch (s) {
    case Status::Fault:
        return "OUTAGE";
    case Status::Suspect:
        return "flagged";
    case Status::Ok:
        return "";
    }
    return "";
}

}  // namespace

void write_report(std::FILE* out, const LoadResult& loaded, const StatsBySensor& stats) {
    std::fprintf(out, "Telemetry report\n");
    std::fprintf(out, "  lines read: %lu   accepted: %lu   rejected: %lu\n",
                 static_cast<unsigned long>(loaded.lines_read),
                 static_cast<unsigned long>(loaded.records.size()),
                 static_cast<unsigned long>(loaded.lines_read - loaded.records.size()));

    if (!loaded.rejected.empty()) {
        std::fprintf(out, "  rejections by reason:\n");
        for (std::map<ParseError, std::size_t>::const_iterator it = loaded.rejected.begin();
             it != loaded.rejected.end(); ++it) {
            std::fprintf(out, "    %-16s %lu\n", to_string(it->first),
                         static_cast<unsigned long>(it->second));
        }
    }

    if (loaded.records.empty()) {
        std::fprintf(out, "  no accepted records\n");
        return;
    }

    const std::pair<double, double> range = value_range(loaded.records);
    std::fprintf(out, "  value range: %.3f .. %.3f\n", range.first, range.second);

    for (StatsBySensor::const_iterator it = stats.begin(); it != stats.end(); ++it) {
        const std::string& name = it->first;
        const SensorStats& s = it->second;
        const SensorConfig* cfg = find_sensor(name);
        const char* units = cfg ? cfg->units : "";
        std::fprintf(out, "\n%s (%s)\n", name.c_str(), units);
        std::fprintf(out, "  n=%lu  min=%.3f  max=%.3f  mean=%.3f  stddev=%.3f  faults=%lu\n",
                     static_cast<unsigned long>(s.count), s.min, s.max, s.mean,
                     std::sqrt(s.variance()), static_cast<unsigned long>(s.faults));

        const std::vector<Record> top = top_n_by_value(loaded.records, name, kTopReadings);
        for (std::size_t i = 0; i < top.size(); ++i) {
            const std::string line = serialize(top[i]);
            std::fprintf(out, "  #%lu %-48s crc=%04X %s\n", static_cast<unsigned long>(i + 1),
                         line.c_str(), crc16(line), severity(top[i].status));
        }
    }
}

}  // namespace telemetry
