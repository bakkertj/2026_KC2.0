// Demo: value categories, verified by the compiler (supplemental deck)
// decltype((expr)) yields T for a prvalue, T& for an lvalue, T&& for an xvalue,
// so every classification on the slides is a static_assert here.
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// [snippet: probe]
#define IS_LVALUE(e)  std::is_lvalue_reference_v<decltype((e))>
#define IS_XVALUE(e)  std::is_rvalue_reference_v<decltype((e))>
#define IS_PRVALUE(e) (!std::is_reference_v<decltype((e))>)
// [/snippet]

struct Widget {
    int n = 0;
    std::string name;
    int&& rref();
    int& lref();
    int val();
};
int&& Widget::rref() { static int x; return std::move(x); }
int& Widget::lref() { static int x; return x; }
int Widget::val() { return 1; }

Widget make() { return {}; }
Widget& global() { static Widget w; return w; }
Widget&& stolen() { return std::move(global()); }

int i = 0;
int arr[3] = {};
Widget w;
int* p = &i;

// [snippet: lvalues]
static_assert(IS_LVALUE(i));          // a name
static_assert(IS_LVALUE(*p));         // dereference
static_assert(IS_LVALUE(arr[1]));     // subscript on an lvalue array
static_assert(IS_LVALUE(w.n));        // member of an lvalue
static_assert(IS_LVALUE(++i));        // pre-increment returns the object
static_assert(IS_LVALUE(i = 5));      // assignment returns the left operand
static_assert(IS_LVALUE("hello"));    // string literal: an lvalue array of const char
static_assert(IS_LVALUE(global()));   // a call returning T&
// [/snippet]

// [snippet: prvalues]
static_assert(IS_PRVALUE(42));           // literal
static_assert(IS_PRVALUE(i + 1));        // arithmetic
static_assert(IS_PRVALUE(i++));          // post-increment returns a copy
static_assert(IS_PRVALUE(make()));       // a call returning by value
static_assert(IS_PRVALUE(Widget{}));     // a temporary
static_assert(IS_PRVALUE(&i));           // address-of
static_assert(IS_PRVALUE([] { return 1; }));   // a lambda expression
static_assert(IS_PRVALUE(i < 3 ? 1 : 2));      // both arms prvalues
// [/snippet]

// [snippet: xvalues]
static_assert(IS_XVALUE(std::move(i)));        // the whole point of std::move
static_assert(IS_XVALUE(static_cast<int&&>(i)));
static_assert(IS_XVALUE(stolen()));            // a call returning T&&
static_assert(IS_XVALUE(make().name));         // member of an rvalue object
static_assert(IS_XVALUE(std::move(arr)[0]));   // subscript on an rvalue array
// [/snippet]

// [snippet: named_rref]
void take(Widget&& x) {
    static_assert(IS_LVALUE(x));               // x has a name: it is an lvalue
    Widget a = x;                               // ...so this COPIES
    Widget b = std::move(x);                    // ...and this moves
    static_assert(IS_XVALUE(std::move(x)));
    (void)a; (void)b;
}
// [/snippet]

// [snippet: overloads]
const char* f(Widget&)        { return "Widget&"; }
const char* f(const Widget&)  { return "const Widget&"; }
const char* f(Widget&&)       { return "Widget&&"; }
// f(w)            -> Widget&         (lvalue)
// f(make())       -> Widget&&        (prvalue, materialized into a temporary)
// f(std::move(w)) -> Widget&&        (xvalue)
// f(cw)           -> const Widget&   (const lvalue)
// [/snippet]

// [snippet: forward]
template <typename T>
void relay(T&& arg) {                    // forwarding reference: T&& on a deduced T
    f(std::forward<T>(arg));             // lvalue in, lvalue out; rvalue in, xvalue out
}
// [/snippet]

#include <cstdio>
int main() {
    const Widget cw;
    std::printf("%s %s %s %s\n", f(w), f(make()), f(std::move(w)), f(cw));
    relay(w);
    relay(make());
    take(make());
}
