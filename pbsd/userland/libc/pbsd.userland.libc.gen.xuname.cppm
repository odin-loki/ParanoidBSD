module;
#include <cstddef>

export module pbsd.userland.libc.gen.xuname;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/gen/__xuname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly xuname_query(char* buf, std::size_t len) noexcept { if (buf == nullptr || len == 0) return status_err(Status::Invalid); buf[0] = '\0'; return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
