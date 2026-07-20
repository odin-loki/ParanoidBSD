module;
#include <cstddef>

export module pbsd.userland.util.hexdump;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/hexdump.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly hexdump_print(const void* data, std::size_t len) noexcept { if (data == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
