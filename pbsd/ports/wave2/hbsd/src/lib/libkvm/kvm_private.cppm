export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_private;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_private.c
// void kvm_private_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_private.c wave=wave2 loc=824
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_private {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_private
