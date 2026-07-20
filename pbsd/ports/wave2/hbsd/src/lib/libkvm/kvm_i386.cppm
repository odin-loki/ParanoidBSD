export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_i386;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_i386.c
// void kvm_i386_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_i386.c wave=wave2 loc=425
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_i386 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_i386
