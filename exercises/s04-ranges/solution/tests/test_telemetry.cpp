// Tests for the Session 4 solution: the Session 3 suite plus ranges-specific
// cases. The report-diff CTest proves the program's behavior is unchanged.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <array>
#include <cstdio>
#include <format>
#include <ranges>
#include <sstream>

#include "telemetry/config.h"
#include "telemetry/crc.h"
#include "telemetry/parser.h"
#include "telemetry/report.h"
#include "telemetry/serialize.h"
#include "telemetry/stats.h"

using namespace telemetry;

// ---- Compile-time checks (Session 3) ---------------------------------------

// The CRC table and crc16 are constexpr: the standard check value is a constant.
static_assert(kCrcTable[0] == 0x0000);
static_assert(kCrcTable[1] == 0x1021);
static_assert(kCrcTable[255] == 0x1EF0);
static_assert(crc16("123456789") == 0x29B1);
static_assert(crc16("") == 0xFFFF);

// The configuration table was validated by a consteval function (config.h);
// lookups of literal names fold to constants.
static_assert(sensors().size() == 6);
static_assert(find_sensor("rpm").has_value());
static_assert(find_sensor("rpm")->max_valid == 12'000.0);
static_assert(!find_sensor("humidity").has_value());
static_assert(find_sensor("temp_core")->in_range(25.0));
static_assert(!find_sensor("temp_core")->in_range(200.0));

// A deliberately broken table is rejected at compile time with a message.
constexpr SensorConfig kBroken[] = {{"a", "x", 0.0, 1.0}, {"a", "x", 0.0, 1.0}};
static_assert(validate(kBroken) == "duplicate sensor name");
constexpr SensorConfig kInverted[] = {{"a", "x", 5.0, 1.0}};
static_assert(validate(kInverted) == "a sensor has min_valid >= max_valid");

// Status helpers are constexpr.
static_assert(to_string(Status::Fault) == "fault");
static_assert(parse_status("suspect") == Status::Suspect);
static_assert(!parse_status("nope").has_value());

// Concepts document what serialize accepts, and static_assert can ask.
static_assert(Serializable<int>);
static_assert(Serializable<double>);
static_assert(Serializable<std::string>);
static_assert(Serializable<const char*>);
static_assert(Serializable<Status>);
static_assert(Serializable<Record>);
static_assert(Serializable<std::vector<int>>);
static_assert(Serializable<std::vector<std::vector<int>>>);
static_assert(!Serializable<ParseError>);   // no overload accepts it, and the concept says so
static_assert(StringLike<std::string_view>);
static_assert(!StringLike<int>);
static_assert(SerializableRange<std::vector<Record>>);
static_assert(!SerializableRange<std::string>);   // strings are StringLike, not ranges, here
static_assert(SerializableRange<decltype(std::views::iota(0, 3))>);   // views are ranges too (Session 4)

// Deducing this: add() chains on lvalues and rvalues alike, and is constexpr.
constexpr SensorStats kThree = SensorStats{}.add(2.0, Status::Ok).add(4.0, Status::Fault).add(6.0, Status::Ok);
static_assert(kThree.count == 3);
static_assert(kThree.mean == 4.0);
static_assert(kThree.min == 2.0 && kThree.max == 6.0);
static_assert(kThree.faults == 1);

TEST_CASE("status names round-trip through optional") {
    CHECK(parse_status("ok") == Status::Ok);
    CHECK(parse_status("fault") == Status::Fault);
    CHECK_FALSE(parse_status("FAULT").has_value());
    CHECK(to_string(Status::Suspect) == "suspect");
    CHECK(std::format("{}", Status::Fault) == "fault");
}

TEST_CASE("records compare by timestamp, sensor, value, status") {
    const Record a{1, "rpm", 100.0, Status::Ok};
    const Record b{2, "rpm", 50.0, Status::Ok};
    const Record c{1, "temp_core", 0.0, Status::Ok};
    const Record d{1, "rpm", 100.0, Status::Fault};
    CHECK(a < b);
    CHECK(a < c);
    CHECK(a < d);
    CHECK(a == a);
    CHECK(a != d);
}

TEST_CASE("Record formats as {ts,\"sensor\",value,status}") {
    const Record r{7, "rpm", 1.5, Status::Ok};
    CHECK(std::format("{}", r) == "{7,\"rpm\",1.500,ok}");
    CHECK(std::format("[{:>24}]", "x") == "[                       x]");  // sanity: std::format itself
}

TEST_CASE("sensor configuration is a span and lookups are optional") {
    CHECK(sensors().size() == 6);
    CHECK(sensors().front().name == "temp_core");
    const auto cfg = find_sensor("voltage_bus");
    REQUIRE(cfg.has_value());
    CHECK(cfg->units == "V");
    CHECK(cfg->max_valid == doctest::Approx(32.0));
    CHECK(cfg->in_range(12.0));
    CHECK_FALSE(cfg->in_range(40.0));
    CHECK_FALSE(find_sensor("humidity").has_value());
    const auto units = [](const SensorConfig& c) { return c.units; };
    CHECK(find_sensor("rpm").transform(units).value_or("none") == "rpm");
    CHECK(find_sensor("nope").transform(units).value_or("none") == "none");
    CHECK(kMaxLineLength == 256);
}

TEST_CASE("crc16 accepts byte spans and string views") {
    CHECK(crc16("123456789") == 0x29B1);
    CHECK(crc16("") == 0xFFFF);
    const std::array<std::byte, 3> bytes{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    CHECK(crc16(bytes) == crc16("abc"));
}

TEST_CASE("split returns views into the original buffer") {
    const std::string text = "a,,c";
    const auto f = split(text, ',');
    REQUIRE(f.size() == 3);
    CHECK(f[0] == "a");
    CHECK(f[1].empty());
    CHECK(f[2] == "c");
    CHECK(f[2].data() == text.data() + 3);  // a view, not a copy
    CHECK(split("", ',').size() == 1);
}

TEST_CASE("parse_record returns a Record on success") {
    const auto r = parse_record("1725000000000,temp_core,41.25,suspect");
    REQUIRE(r.has_value());
    CHECK(r->ts == 1725000000000LL);
    CHECK(r->sensor == "temp_core");
    CHECK(r->value == doctest::Approx(41.25));
    CHECK(r->status == Status::Suspect);

    const auto s = parse_record("5,rpm,4800");
    REQUIRE(s);
    CHECK(s->status == Status::Ok);
}

TEST_CASE("parse_record returns the reason on failure") {
    struct Case {
        std::string_view line;
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
    for (const auto& [line, expected] : cases) {
        CAPTURE(line);
        const auto r = parse_record(line);
        REQUIRE_FALSE(r.has_value());
        CHECK(r.error() == expected);
    }
    const std::string too_long = "1,temp_core," + std::string(300, '1');
    CHECK(parse_record(too_long).error() == ParseError::LineTooLong);
}

TEST_CASE("expected composes: value_or and error_or") {
    const Record fallback{0, "none", 0.0, Status::Fault};
    CHECK(parse_record("bad").value_or(fallback).sensor == "none");
    CHECK(parse_record("1,rpm,10").value_or(fallback).sensor == "rpm");
    CHECK(parse_record("1,rpm,10").error_or(ParseError::EmptyLine) == ParseError::EmptyLine);
    CHECK(std::format("{}", parse_record("").error()) == "empty line");
}

TEST_CASE("load_stream counts rejections by reason and strips CR") {
    std::istringstream in("1,rpm,10\r\n\n2,rpm,20\nbad\n3,humidity,1\n4,humidity,2\n");
    const auto res = load_stream(in);
    CHECK(res.lines_read == 6);
    REQUIRE(res.records.size() == 2);
    CHECK(res.records[1].value == doctest::Approx(20.0));
    CHECK(res.rejected.size() == 3);
    CHECK(res.rejected.at(ParseError::EmptyLine) == 1);
    CHECK(res.rejected.at(ParseError::MissingField) == 1);
    CHECK(res.rejected.at(ParseError::UnknownSensor) == 2);
}

TEST_CASE("compute_stats produces Welford statistics per sensor") {
    const std::vector<Record> v{
        {1, "rpm", 2.0, Status::Ok}, {2, "rpm", 4.0, Status::Ok},    {3, "rpm", 4.0, Status::Ok},
        {4, "rpm", 4.0, Status::Ok}, {5, "rpm", 5.0, Status::Fault}, {6, "rpm", 5.0, Status::Ok},
        {7, "rpm", 7.0, Status::Ok}, {8, "rpm", 9.0, Status::Ok},    {1, "pressure", 100.0, Status::Ok},
    };
    const auto s = compute_stats(v);
    REQUIRE(s.size() == 2);
    const auto& rpm = s.find("rpm")->second;  // find by string literal: transparent comparator
    CHECK(rpm.count == 8);
    CHECK(rpm.min == doctest::Approx(2.0));
    CHECK(rpm.max == doctest::Approx(9.0));
    CHECK(rpm.mean == doctest::Approx(5.0));
    CHECK(rpm.variance() == doctest::Approx(4.0));
    CHECK(rpm.faults == 1);
    CHECK(s.find("pressure")->second.variance() == doctest::Approx(0.0));
}

TEST_CASE("span parameters accept arrays, vectors, and sub-ranges") {
    const Record arr[] = {{1, "rpm", 30.0, Status::Ok}, {2, "rpm", 10.0, Status::Ok},
                          {3, "pressure", 99.0, Status::Ok}, {4, "rpm", 20.0, Status::Ok},
                          {5, "rpm", 30.0, Status::Ok}};
    const auto r = value_range(arr);
    REQUIRE(r.has_value());
    CHECK(r->first == doctest::Approx(10.0));
    CHECK(r->second == doctest::Approx(99.0));
    CHECK_FALSE(value_range({}).has_value());

    const std::span<const Record> first_two{arr, 2};
    CHECK(value_range(first_two)->second == doctest::Approx(30.0));

    const auto top = top_n_by_value(arr, "rpm", 3);
    REQUIRE(top.size() == 3);
    CHECK(top[0].ts == 1);  // ties keep input order
    CHECK(top[1].ts == 5);
    CHECK(top[2].ts == 4);
    CHECK(top_n_by_value(arr, "pressure", 5).size() == 1);
    CHECK(top_n_by_value(arr, "voltage_bus", 5).empty());
}

TEST_CASE("first_at_or_after on a sorted span") {
    const std::vector<Record> v{{10, "rpm", 0.0, Status::Ok}, {20, "rpm", 0.0, Status::Ok},
                                {30, "rpm", 0.0, Status::Ok}};
    CHECK(first_at_or_after(v, 5) == 0);
    CHECK(first_at_or_after(v, 20) == 1);
    CHECK(first_at_or_after(v, 21) == 2);
    CHECK(first_at_or_after(v, 31) == 3);
}

TEST_CASE("serialize accepts views, not just containers (Session 4)") {
    const std::vector<int> v{1, 2, 3, 4};
    CHECK(serialize(v | std::views::take(2)) == "[1, 2]");
    CHECK(serialize(v | std::views::filter([](int x) { return x % 2 == 0; })) == "[2, 4]");
    CHECK(serialize(std::views::iota(1, 4)) == "[1, 2, 3]");
}

TEST_CASE("split matches the loop's contract on edge cases (Session 4)") {
    CHECK(split("", ',').size() == 1);              // views::split alone would give 0
    CHECK(split(",", ',').size() == 2);
    CHECK(split("a,", ',').back().empty());
    CHECK(split(",a", ',').front().empty());
}

TEST_CASE("value_range and top_n use projections (Session 4)") {
    const std::vector<Record> v{{3, "rpm", 5.0, Status::Ok}, {1, "rpm", 9.0, Status::Ok}, {2, "rpm", 1.0, Status::Ok}};
    const auto r = value_range(v);
    REQUIRE(r);
    CHECK(r->first == doctest::Approx(1.0));
    CHECK(r->second == doctest::Approx(9.0));
    const auto top = top_n_by_value(v, "rpm", 2);
    REQUIRE(top.size() == 2);
    CHECK(top[0].ts == 1);
    CHECK(top[1].ts == 3);
}

TEST_CASE("serialize_all folds any number of arguments") {
    CHECK(serialize_all(1, 2.5, "x", Status::Ok) == "[1, 2.500, \"x\", ok]");
    CHECK(serialize_all() == "[]");
    CHECK(serialize_all(std::vector<int>{1, 2}, 3) == "[[1, 2], 3]");
}

TEST_CASE("add chains on an lvalue and returns a reference to it") {
    SensorStats s;
    s.add(1.0, Status::Ok).add(3.0, Status::Ok);
    CHECK(s.count == 2);
    CHECK(s.mean == doctest::Approx(2.0));
    CHECK(&s.add(5.0, Status::Ok) == &s);   // lvalue in, SensorStats& out
}

TEST_CASE("serialize formats scalars, records, vectors and spans") {
    CHECK(serialize(42) == "42");
    CHECK(serialize(1725000000000LL) == "1725000000000");
    CHECK(serialize(3.14159) == "3.142");
    CHECK(serialize("x") == "\"x\"");
    CHECK(serialize(Status::Fault) == "fault");
    CHECK(serialize(Record{7, "rpm", 1.5, Status::Ok}) == "{7,\"rpm\",1.500,ok}");
    CHECK(serialize(std::vector<int>{1, 2}) == "[1, 2]");
    const int arr[] = {3, 4, 5};
    CHECK(serialize(std::span<const int>{arr}) == "[3, 4, 5]");
}

TEST_CASE("write_report produces the expected layout") {
    std::istringstream in("1,rpm,100\n2,rpm,300,fault\n3,rpm,200\nbad\n");
    const auto loaded = load_stream(in);
    const auto stats = compute_stats(loaded.records);

    std::FILE* f = std::tmpfile();
    REQUIRE(f != nullptr);
    write_report(f, loaded, stats);
    std::rewind(f);
    std::string text;
    char buf[512];
    while (std::fgets(buf, sizeof buf, f)) text += buf;
    std::fclose(f);

    CHECK(text.contains("lines read: 4   accepted: 3   rejected: 1"));
    CHECK(text.contains("missing field    1"));
    CHECK(text.contains("value range: 100.000 .. 300.000"));
    CHECK(text.contains("rpm (rpm)"));
    CHECK(text.contains("n=3  min=100.000  max=300.000  mean=200.000"));
    CHECK(text.contains("faults=1"));
    CHECK(text.contains("#1 {2,\"rpm\",300.000,fault}"));
    CHECK(text.contains("OUTAGE"));
}
