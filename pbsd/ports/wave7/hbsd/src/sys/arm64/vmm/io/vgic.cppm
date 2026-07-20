export module pbsd.port.wave7.hbsd.src.sys.arm64.vmm.io.vgic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/vmm/io/vgic.c
// void vgic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/vmm/io/vgic.c wave=wave7 loc=122
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::vmm::io::vgic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::vmm::io::vgic
