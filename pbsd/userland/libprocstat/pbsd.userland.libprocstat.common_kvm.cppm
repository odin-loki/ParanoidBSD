module;
#include <cstddef>

export module pbsd.userland.libprocstat.common_kvm;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/common_kvm.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_kvm_attach() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
