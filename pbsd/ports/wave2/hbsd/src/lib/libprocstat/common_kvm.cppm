export module pbsd.port.wave2.hbsd.src.lib.libprocstat.common_kvm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libprocstat/common_kvm.c
// void common_kvm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libprocstat/common_kvm.c wave=wave2 loc=212
export namespace pbsd::port::wave2::hbsd::src::lib::libprocstat::common_kvm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libprocstat::common_kvm
