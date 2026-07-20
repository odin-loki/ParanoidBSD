export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_cptime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_cptime.c
// void kvm_cptime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_cptime.c wave=wave2 loc=139
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_cptime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_cptime
