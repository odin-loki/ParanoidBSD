module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.siginterrupt;

export import pbsd.core;

/// siginterrupt from hbsd/src/lib/libc/gen/siginterrupt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status siginterrupt_sig(int sig, int flag) noexcept { (void)sig; (void)flag; return Status::Ok; }

} // namespace pbsd::userland::libc
