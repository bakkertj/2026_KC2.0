# Cheat sheet: vocabulary types (Session 2)

One page for the types that replace raw pointers, sentinel values, out-parameters, and `printf`. Badge in brackets is the standard that introduced it.

## Which type

| You need | Use | Not |
|---|---|---|
| Read-only text parameter | `std::string_view` by value [17] | `const std::string&`, `const char*` |
| Read-only contiguous elements | `std::span<const T>` by value [20] | `const T*` plus a length, `const std::vector<T>&` |
| A buffer to write into | `std::span<T>` [20] | `T*` plus a length |
| A value you will keep | `T` by value, then `std::move` it | `const T&` plus a copy |
| Maybe one `T` | `std::optional<T>` [17] | `T*`, sentinel values (`-1`, `""`), `std::pair<T, bool>` |
| A `T` or a reason there is none | `std::expected<T, E>` [23], `[[nodiscard]]` | `optional` (loses the reason), `bool` plus out-parameter, exceptions for expected failures |
| Exactly one of a fixed set | `std::variant<A, B, C>` [17] | tag plus union, `void*`, base class plus `dynamic_cast` |
| One of an open set, fixed operations | virtual base class | `variant` |
| Anything, checked at runtime | `std::any` [17] | `void*` |
| Someone else's `T`, maybe absent | `T*` non-owning | `optional<T&>` (not in C++23), `shared_ptr` |
| Someone else's `T`, never absent | `T&` | `T*` |
| Owned, one owner | `std::unique_ptr<T>` [11/14] | raw owning pointer, `shared_ptr` by default |
| Owned, shared for real | `std::shared_ptr<T>` | anything you could express with `unique_ptr` |
| A line of text output | `std::format` [20] / `std::print` [23] with a `std::formatter` | `printf`, `iostream` |

## The one rule for views

`string_view`, `span`, and `mdspan` are a pointer and a length. They own nothing.

- Never store one unless you own what it points at and outlive it.
- Never return one into a local or a temporary (`std::string_view(s).substr(...)` of a local `s` dangles on return).
- `string_view` is not NUL-terminated: do not pass `.data()` to a C API; copy to a `std::string` first.
- `span<T>` converts to `span<const T>`; `span<T, N>` (static extent) is a compile-time size check at hardware boundaries; `std::as_bytes` / `as_writable_bytes` for byte views.
- Keep `const std::string&` only when the callee needs a `std::string` anyway (stores it, calls `c_str()`, or passes it on as a `string`).

## `optional` and `expected` in one minute

| Operation | `optional<T>` | `expected<T, E>` |
|---|---|---|
| Test | `if (o)`, `o.has_value()` | `if (e)`, `e.has_value()` |
| Read (checked) | `o.value()` throws `bad_optional_access` | `e.value()` throws `bad_expected_access<E>` |
| Read (unchecked) | `*o`, `o->m` | `*e`, `e->m` |
| Fallback | `o.value_or(x)` | `e.value_or(x)`, `e.error_or(err)` |
| Error | none | `e.error()`, `std::unexpected(err)` to construct |
| Map, can't fail | `o.transform(f)` [23] | `e.transform(f)` |
| Map, can fail | `o.and_then(f)` (f returns `optional`) [23] | `e.and_then(f)` (f returns `expected`) |
| Recover | `o.or_else(g)` [23] | `e.or_else(g)`, `e.transform_error(h)` |
| Nothing to return | `std::nullopt` | `std::expected<void, E>{}` on success |

- `transform(&T::member)` does not compile: the pointer-to-member yields a reference and `optional<T&>` is ill-formed. Use a lambda.
- Neither type is `[[nodiscard]]` by itself. Put `[[nodiscard]]` on every function that returns one.
- Error type: a small `enum class` or a struct with a code and a message. Convert at API boundaries; do not propagate strings.
- `expected` for failures the caller is expected to handle (parse errors, lookups). Exceptions for failures nobody local can handle.

## `variant` and `visit`

- `std::holds_alternative<T>(v)`, `std::get<T>(v)` (throws `bad_variant_access`), `std::get_if<T>(&v)` (returns pointer or null), `v.index()`.
- Visitor pattern: `template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };` plus, in C++17 only, the deduction guide `template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;` (C++20 deduces it). Then `std::visit(overloaded{ [](A&){...}, [](B&){...} }, v)`.
- A visitor that misses an alternative fails to compile, unless another lambda accepts it by implicit conversion (`float` to a `double` lambda). Prefer exact-type lambdas or `auto&&` as a deliberate catch-all.
- Default-constructed `variant` holds the first alternative; use `std::monostate` first if none is default-constructible.
- `valueless_by_exception()` only after a throwing move or copy during assignment; most code can ignore it.

## `format` and `print`

`std::format(fmt, args...)` returns a `std::string` [20]. `std::print` / `std::println` write it [23]. The spec goes after the colon: `{:<10}` left-align width 10, `{:>8}` right-align, `{:08.3f}` zero-pad width 8 precision 3, `{:x}` hex, `{:#b}` binary with prefix, `{}` for any type with a `formatter`. Positional: `{0} {1} {0}`.

| From | To |
|---|---|
| `printf("%d %s %.2f", i, s.c_str(), d)` | `print("{} {} {:.2f}", i, s, d)` |
| `printf("%-10s|%5d", ...)` | `print("{:<10}|{:>5}", ...)` |
| `std::cout << std::hex << x` | `print("{:x}", x)` (no sticky state) |
| `std::ostringstream` | `std::format` |
| `snprintf(buf, n, ...)` | `std::format_to_n(buf, n, ...)`; `.out - buf` is the length written, `.size` the length wanted |

Your own type: specialize `template <> struct std::formatter<MyType>` with `constexpr auto parse(auto& ctx)` and `auto format(const MyType& v, auto& ctx) const` that returns `std::format_to(ctx.out(), ...)`. Inherit from `std::formatter<std::string_view>` to reuse the standard spec. Ranges format as `[a, b, c]` from C++23 (GCC 15; libc++ 18).

`clang-tidy modernize-use-std-print` converts `printf` calls; `modernize-use-std-format` converts `snprintf` into `format`.

## Library tour, by standard

- **C++17:** `std::filesystem` (`path`, `exists`, `create_directories`, `directory_iterator`); `std::byte`; `to_chars` / `from_chars` (no locale, no errno, no allocation); `std::invoke`, `std::apply`; `std::clamp`, `std::gcd`, `std::lcm`; map `try_emplace`, `insert_or_assign`, `extract`, `merge`; `std::size`, `std::data`, `std::empty`; transparent comparators `std::less<>` [14] let `std::map<std::string, T>` find by `string_view` (for `unordered_map` you also need a transparent hash and `std::equal_to<>`).
- **C++20:** `std::bit_cast`, `<bit>` (`popcount`, `countl_zero`, `has_single_bit`, `rotl`); `std::source_location`; `std::erase` / `erase_if` for every container; `starts_with` / `ends_with`; `contains` on associative containers; `<chrono>` calendars and time zones; `std::numbers::pi`; `std::midpoint`, `std::lerp`; `std::to_array`.
- **C++23:** `std::flat_map` / `flat_set` (sorted vectors behind a map interface; GCC 15, libc++ 20 / 21); `std::stacktrace` (GCC, link `-lstdc++exp`); `std::move_only_function` (libstdc++ only); `std::string::contains`; `std::byteswap`; `std::out_ptr` / `inout_ptr` for `T**` C APIs; `std::forward_like`; `resize_and_overwrite`; `std::mdspan` (libc++ 17, GCC 15).

## Deprecated and removed

Deprecated in C++17 and still present: `std::iterator`, `<codecvt>` (removed in C++26 with `strstream`). Deprecated in C++17, removed in C++20: `std::result_of`, `std::uncaught_exception`, `<ciso646>`. Find them with `-Wdeprecated-declarations` (on by default on GCC and Clang).

## Toolchain notes for this session

`std::expected` needs libstdc++ 12+ or libc++ 16+; Clang with libstdc++ lacks it before Clang 19 (concepts macro). `from_chars` for `double` needs libstdc++ 11+ or libc++ 20+. `std::print` needs GCC 14 or libc++ 17. Full table: `handouts/toolchain-support-matrix.md`.
