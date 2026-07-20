export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm.c
// void kvm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm.c wave=wave2 loc=536
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm
