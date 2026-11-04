module;
#include <cstddef>

export module pbsd.userland.util.expand_number;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/expand_number.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly expand_number_parse(const char* buf, long long* num) noexcept { if (buf == nullptr || num == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
