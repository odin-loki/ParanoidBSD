module;

export module pbsd.userland.libc.gen.cap;

/// cap_sandboxed from hbsd/src/lib/libc/gen/cap_sandboxed.c
export namespace pbsd::userland::libc {

inline bool g_cap_sandboxed = false;

[[nodiscard]] inline bool cap_sandboxed() noexcept { return g_cap_sandboxed; }

inline void set_cap_sandboxed(bool v) noexcept { g_cap_sandboxed = v; }

} // namespace pbsd::userland::libc
