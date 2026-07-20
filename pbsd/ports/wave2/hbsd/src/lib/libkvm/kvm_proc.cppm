export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_proc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_proc.c
// void kvm_proc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_proc.c wave=wave2 loc=791
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_proc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_proc
