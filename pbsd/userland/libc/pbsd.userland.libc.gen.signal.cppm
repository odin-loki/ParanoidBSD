module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.signal;

export import pbsd.core;

/// signal from hbsd/src/lib/libc/gen/signal.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status signal_num(int sig) noexcept { return sig > 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
