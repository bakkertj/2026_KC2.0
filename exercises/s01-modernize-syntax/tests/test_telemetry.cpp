#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "telemetry.h"

TEST_CASE("parse_record accepts well-formed lines") {
    Record r;
    REQUIRE(parse_record("1700000000,temp1,21.5", &r));
    CHECK(r.ts == 1700000000);
    CHECK(r.sensor == "temp1");
    CHECK(r.value == doctest::Approx(21.5));
}

TEST_CASE("parse_record rejects malformed lines") {
    Record r;
    CHECK_FALSE(parse_record("notanumber,temp1,21.5", &r));
    CHECK_FALSE(parse_record("1,temp1", &r));
    CHECK_FALSE(parse_record("1,temp1,abc", &r));
}

TEST_CASE("count_by_sensor counts occurrences") {
    std::vector<Record> v = {{1, "a", 0.0}, {2, "b", 0.0}, {3, "a", 0.0}};
    SensorCounts c = count_by_sensor(v);
    CHECK(c["a"] == 2);
    CHECK(c["b"] == 1);
}

TEST_CASE("value_range finds min and max") {
    std::vector<Record> v = {{1, "a", 5.0}, {2, "a", -1.0}, {3, "a", 9.5}};
    std::pair<double, double> r = value_range(v);
    CHECK(r.first == doctest::Approx(-1.0));
    CHECK(r.second == doctest::Approx(9.5));
}

TEST_CASE("records order by timestamp first") {
    Record a = {1, "z", 0.0}, b = {2, "a", 0.0};
    CHECK(a < b);
    CHECK(a == a);
}
