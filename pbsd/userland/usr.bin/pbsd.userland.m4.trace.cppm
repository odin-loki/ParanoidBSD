module;

export module pbsd.userland.m4.trace;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/trace.c
export namespace pbsd::userland::usr_bin::m4::trace {

[[nodiscard]] inline bool trace_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::trace
