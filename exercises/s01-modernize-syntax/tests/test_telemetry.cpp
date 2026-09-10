// The same tests run against starter/ and solution/. They are written in
// C++11 so the starter can be compiled as C++11 to prove it really is C++11.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstdio>
#include <sstream>

#include "telemetry/config.h"
#include "telemetry/crc.h"
#include "telemetry/parser.h"
#include "telemetry/report.h"
#include "telemetry/serialize.h"
#include "telemetry/stats.h"

using namespace telemetry;

namespace {

Record make(Timestamp ts, const char* sensor, double value, Status status = Status::Ok) {
    Record r;
    r.ts = ts;
    r.sensor = sensor;
    r.value = value;
    r.status = status;
    return r;
}

}  // namespace

TEST_CASE("status names round-trip") {
    Status s;
    CHECK(status_from_string("ok", &s));
    CHECK(s == Status::Ok);
    CHECK(status_from_string("fault", &s));
    CHECK(s == Status::Fault);
    CHECK_FALSE(status_from_string("FAULT", &s));
    CHECK(std::string(to_string(Status::Suspect)) == "suspect");
}

TEST_CASE("records compare by timestamp, sensor, value, status") {
    const Record a = make(1, "rpm", 100.0);
    const Record b = make(2, "rpm", 50.0);
    const Record c = make(1, "temp_core", 0.0);
    const Record d = make(1, "rpm", 100.0, Status::Fault);
    CHECK(a < b);
    CHECK(a < c);
    CHECK(a < d);
    CHECK(a == a);
    CHECK(a != d);
    CHECK(b > a);
    CHECK(a <= a);
    CHECK(d >= a);
}

TEST_CASE("sensor configuration lookup") {
    CHECK(sensor_count() == 6);
    const SensorConfig* cfg = find_sensor("voltage_bus");
    REQUIRE(cfg != nullptr);
    CHECK(std::string(cfg->units) == "V");
    CHECK(cfg->max_valid == doctest::Approx(32.0));
    CHECK(find_sensor("humidity") == nullptr);
    CHECK(kMaxLineLength == 256);
}

TEST_CASE("crc16 matches the CCITT-FALSE check value") {
    // The standard check value for "123456789" under CRC-16/CCITT-FALSE.
    CHECK(crc16("123456789") == 0x29B1);
    CHECK(crc16("") == 0xFFFF);
    CHECK(crc16("a") != crc16("b"));
}

TEST_CASE("split keeps empty fields") {
    const std::vector<std::string> f = split("a,,c", ',');
    REQUIRE(f.size() == 3);
    CHECK(f[0] == "a");
    CHECK(f[1] == "");
    CHECK(f[2] == "c");
    CHECK(split("", ',').size() == 1);
}

TEST_CASE("parse_record accepts a well-formed line") {
    Record r;
    ParseError err;
    REQUIRE(parse_record("1725000000000,temp_core,41.25,suspect", &r, &err));
    CHECK(err == ParseError::None);
    CHECK(r.ts == 1725000000000LL);
    CHECK(r.sensor == "temp_core");
    CHECK(r.value == doctest::Approx(41.25));
    CHECK(r.status == Status::Suspect);

    REQUIRE(parse_record("5,rpm,4800", &r, &err));
    CHECK(r.status == Status::Ok);
}

TEST_CASE("parse_record reports each failure reason") {
    Record r;
    ParseError err;
    struct Case {
        const char* line;
        ParseError expected;
    };
    const Case cases[] = {
        {"", ParseError::EmptyLine},
        {"1,temp_core", ParseError::MissingField},
        {"abc,temp_core,1.0", ParseError::BadTimestamp},
        {"12x,temp_core,1.0", ParseError::BadTimestamp},
        {"1,humidity,1.0", ParseError::UnknownSensor},
        {"1,temp_core,warm", ParseError::BadValue},
        {"1,temp_core,", ParseError::BadValue},
        {"1,temp_core,500", ParseError::OutOfRange},
        {"1,temp_core,1.0,broken", ParseError::UnknownStatus},
    };
    for (std::size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        CAPTURE(cases[i].line);
        CHECK_FALSE(parse_record(cases[i].line, &r, &err));
        CHECK(err == cases[i].expected);
    }
    const std::string too_long = "1,temp_core," + std::string(300, '1');
    CHECK_FALSE(parse_record(too_long, &r, &err));
    CHECK(err == ParseError::LineTooLong);
}

TEST_CASE("load_stream counts rejections by reason and strips CR") {
    std::istringstream in("1,rpm,10\r\n\n2,rpm,20\nbad\n3,humidity,1\n4,humidity,2\n");
    const LoadResult res = load_stream(in);
    CHECK(res.lines_read == 6);
    REQUIRE(res.records.size() == 2);
    CHECK(res.records[1].value == doctest::Approx(20.0));
    CHECK(res.rejected.size() == 3);
    CHECK(res.rejected.find(ParseError::EmptyLine)->second == 1);
    CHECK(res.rejected.find(ParseError::MissingField)->second == 1);
    CHECK(res.rejected.find(ParseError::UnknownSensor)->second == 2);
}

TEST_CASE("compute_stats produces Welford statistics per sensor") {
    std::vector<Record> v;
    v.push_back(make(1, "rpm", 2.0));
    v.push_back(make(2, "rpm", 4.0));
    v.push_back(make(3, "rpm", 4.0));
    v.push_back(make(4, "rpm", 4.0));
    v.push_back(make(5, "rpm", 5.0, Status::Fault));
    v.push_back(make(6, "rpm", 5.0));
    v.push_back(make(7, "rpm", 7.0));
    v.push_back(make(8, "rpm", 9.0));
    v.push_back(make(1, "pressure", 100.0));

    const StatsBySensor s = compute_stats(v);
    REQUIRE(s.size() == 2);
    const SensorStats& rpm = s.find("rpm")->second;
    CHECK(rpm.count == 8);
    CHECK(rpm.min == doctest::Approx(2.0));
    CHECK(rpm.max == doctest::Approx(9.0));
    CHECK(rpm.mean == doctest::Approx(5.0));
    CHECK(rpm.variance() == doctest::Approx(4.0));
    CHECK(rpm.faults == 1);
    const SensorStats& p = s.find("pressure")->second;
    CHECK(p.count == 1);
    CHECK(p.variance() == doctest::Approx(0.0));
}

TEST_CASE("value_range and top_n_by_value") {
    std::vector<Record> v;
    v.push_back(make(1, "rpm", 30.0));
    v.push_back(make(2, "rpm", 10.0));
    v.push_back(make(3, "pressure", 99.0));
    v.push_back(make(4, "rpm", 20.0));
    v.push_back(make(5, "rpm", 30.0));

    const std::pair<double, double> r = value_range(v);
    CHECK(r.first == doctest::Approx(10.0));
    CHECK(r.second == doctest::Approx(99.0));

    const std::vector<Record> top = top_n_by_value(v, "rpm", 3);
    REQUIRE(top.size() == 3);
    CHECK(top[0].ts == 1);  // ties keep input order
    CHECK(top[1].ts == 5);
    CHECK(top[2].ts == 4);
    CHECK(top_n_by_value(v, "pressure", 5).size() == 1);
    CHECK(top_n_by_value(v, "voltage_bus", 5).empty());
}

TEST_CASE("first_at_or_after on a sorted vector") {
    std::vector<Record> v;
    v.push_back(make(10, "rpm", 0.0));
    v.push_back(make(20, "rpm", 0.0));
    v.push_back(make(30, "rpm", 0.0));
    CHECK(first_at_or_after(v, 5) == 0);
    CHECK(first_at_or_after(v, 20) == 1);
    CHECK(first_at_or_after(v, 21) == 2);
    CHECK(first_at_or_after(v, 31) == 3);
}

TEST_CASE("serialize formats scalars, records and vectors") {
    CHECK(serialize(42) == "42");
    CHECK(serialize(1725000000000LL) == "1725000000000");
    CHECK(serialize(3.14159) == "3.142");
    CHECK(serialize(std::string("x")) == "\"x\"");
    CHECK(serialize(Status::Fault) == "fault");
    CHECK(serialize(make(7, "rpm", 1.5)) == "{7,\"rpm\",1.500,ok}");
    std::vector<int> ints;
    ints.push_back(1);
    ints.push_back(2);
    CHECK(serialize(ints) == "[1, 2]");
}

TEST_CASE("write_report produces the expected layout") {
    std::istringstream in("1,rpm,100\n2,rpm,300,fault\n3,rpm,200\nbad\n");
    const LoadResult loaded = load_stream(in);
    const StatsBySensor stats = compute_stats(loaded.records);

    std::FILE* f = std::tmpfile();
    REQUIRE(f != nullptr);
    write_report(f, loaded, stats);
    std::rewind(f);
    std::string text;
    char buf[512];
    while (std::fgets(buf, sizeof buf, f)) text += buf;
    std::fclose(f);

    CHECK(text.find("lines read: 4   accepted: 3   rejected: 1") != std::string::npos);
    CHECK(text.find("missing field    1") != std::string::npos);
    CHECK(text.find("value range: 100.000 .. 300.000") != std::string::npos);
    CHECK(text.find("rpm (rpm)") != std::string::npos);
    CHECK(text.find("n=3  min=100.000  max=300.000  mean=200.000") != std::string::npos);
    CHECK(text.find("faults=1") != std::string::npos);
    CHECK(text.find("#1 {2,\"rpm\",300.000,fault}") != std::string::npos);
    CHECK(text.find("OUTAGE") != std::string::npos);
}
