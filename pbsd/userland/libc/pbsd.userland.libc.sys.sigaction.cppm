module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigaction;

export import pbsd.core;

/// sigaction from hbsd/src/lib/libc/sys/sigaction.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigaction_sig(int sig) noexcept { return sig > 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
