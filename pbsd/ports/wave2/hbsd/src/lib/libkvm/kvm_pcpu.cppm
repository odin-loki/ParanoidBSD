export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_pcpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_pcpu.c
// void kvm_pcpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_pcpu.c wave=wave2 loc=382
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_pcpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_pcpu
