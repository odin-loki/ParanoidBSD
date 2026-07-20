export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_getloadavg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_getloadavg.c
// void kvm_getloadavg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_getloadavg.c wave=wave2 loc=98
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_getloadavg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_getloadavg
