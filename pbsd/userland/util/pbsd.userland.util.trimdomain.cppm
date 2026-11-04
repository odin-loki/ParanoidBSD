module;
#include <cstddef>

export module pbsd.userland.util.trimdomain;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/trimdomain.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly trimdomain_host(char* host, std::size_t len) noexcept { if (host == nullptr || len == 0) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
