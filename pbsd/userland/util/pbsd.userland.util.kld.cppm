module;
#include <cstddef>

export module pbsd.userland.util.kld;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/kld.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly kld_load(const char* name) noexcept { if (name == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
