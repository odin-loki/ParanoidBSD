export module pbsd.port.wave2.hbsd.src.lib.libutil.cpuset;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/cpuset.c
// void cpuset_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/cpuset.c wave=wave2 loc=187
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::cpuset {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::cpuset
