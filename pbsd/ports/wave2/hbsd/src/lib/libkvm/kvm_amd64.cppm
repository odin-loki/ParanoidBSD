export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_amd64;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_amd64.c
// void kvm_amd64_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_amd64.c wave=wave2 loc=360
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_amd64 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_amd64
