// Demo: monadic std::expected (C++23)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/77Y1GPfxa
#include <cstdio>
#include <expected>
#include <string>
#include <string_view>

enum class Error { Empty, BadNumber, OutOfRange, Io };

std::expected<std::string_view, Error> read_field(std::string_view line) {
    if (line.empty()) return std::unexpected(Error::Empty);
    return line.substr(0, line.find(','));
}
std::expected<int, Error> to_int(std::string_view f) {
    if (f == "42") return 42;
    return std::unexpected(Error::BadNumber);
}
std::expected<int, Error> check_range(int v) {
    if (v > 100) return std::unexpected(Error::OutOfRange);
    return v;
}

// [snippet: chain]
// Each stage may fail; the first failure short-circuits the rest.
std::expected<double, Error> scaled(std::string_view line) {
    return read_field(line)
        .and_then(to_int)                              // expected<sv> -> expected<int>
        .and_then(check_range)                         // expected<int> -> expected<int>
        .transform([](int v) { return v * 0.5; });     // cannot fail: plain value in, wrapped out
}

// Map an inner error type to an outer one at a layer boundary.
enum class ApiError { Parse, Io };
std::expected<double, ApiError> api(std::string_view line) {
    return scaled(line).transform_error([](Error e) {
        return e == Error::Io ? ApiError::Io : ApiError::Parse;
    });
}
// [/snippet]

// [snippet: before]
// The same pipeline without expected: four ifs, three out-params, or exceptions
// bool scaled_cpp11(const std::string& line, double* out, Error* err) {
//     std::string field; if (!read_field(line, &field, err)) return false;
//     int v;             if (!to_int(field, &v, err))        return false;
//                        if (!check_range(v, err))           return false;
//     *out = v * 0.5;    return true;
// }
// [/snippet]

int main() {
    std::printf("%f %d %d\n", scaled("42,x").value_or(-1), scaled("").has_value(),
                static_cast<int>(api("7").error()));
}
