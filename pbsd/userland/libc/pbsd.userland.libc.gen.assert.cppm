module;

export module pbsd.userland.libc.gen.assert;

/// __assert hook from hbsd/src/lib/libc/gen/assert.c (logic-only; no stderr I/O)
export namespace pbsd::userland::libc {

using AssertHandler = void (*)(const char* func, const char* file, int line,
                               const char* failedexpr) noexcept;

inline AssertHandler g_assert_handler = nullptr;

inline void set_assert_handler(AssertHandler h) noexcept { g_assert_handler = h; }

[[nodiscard]] inline AssertHandler assert_handler() noexcept { return g_assert_handler; }

inline void report_assert_failure(const char* func, const char* file, int line,
                                  const char* failedexpr) noexcept {
    if (g_assert_handler != nullptr) {
        g_assert_handler(func, file, line, failedexpr);
    }
}

} // namespace pbsd::userland::libc
