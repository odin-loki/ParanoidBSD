export module pbsd.port.wave2.hbsd.src.lib.libkvm.kvm_getswapinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/kvm_getswapinfo.c
// void kvm_getswapinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/kvm_getswapinfo.c wave=wave2 loc=267
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_getswapinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::kvm_getswapinfo
