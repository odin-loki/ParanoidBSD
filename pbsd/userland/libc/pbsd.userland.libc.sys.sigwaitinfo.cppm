module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigwaitinfo;

export import pbsd.core;

/// sigwaitinfo from hbsd/src/lib/libc/sys/sigwaitinfo.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigwaitinfo_set() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
