export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_arm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_arm.c
// void kvm_arm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_arm.c wave=wave2 loc=277
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_arm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_arm
