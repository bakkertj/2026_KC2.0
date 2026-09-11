# Starter is the Session 4 solution. The solution splits parsing and statistics
# across a producer and a consumer thread (jthread, stop_token, counting_semaphore),
# and exposes the parser as a std::generator where the library has one.
# Report stays byte-identical. Build with -DCOURSE_SANITIZE=thread to run the
# tests under ThreadSanitizer.
add_exercise_variant(s05 starter 23)
add_exercise_variant(s05 solution 23)
add_report_diff_test(s05)
