# Starter is the Session 2 solution. The solution moves work to compile time
# (constexpr CRC table, consteval config validation) and replaces the serialize
# overload set with constrained templates. Report must stay byte-identical.
add_exercise_variant(s03 starter 23)
add_exercise_variant(s03 solution 23)
add_report_diff_test(s03)
