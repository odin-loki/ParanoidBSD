module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.pdfork;

export import pbsd.core;

/// pdfork from hbsd/src/lib/libc/sys/pdfork.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pdfork_flags(int flags) noexcept { (void)flags; return Status::Ok; }

} // namespace pbsd::userland::libc
