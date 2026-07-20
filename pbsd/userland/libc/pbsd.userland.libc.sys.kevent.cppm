module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.kevent;

export import pbsd.core;

/// kevent from hbsd/src/lib/libc/sys/kevent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status kevent_kq(int kq) noexcept { return kq >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
