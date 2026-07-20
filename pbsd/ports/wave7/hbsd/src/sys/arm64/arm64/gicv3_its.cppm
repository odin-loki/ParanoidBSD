export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.gicv3_its;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/gicv3_its.c
// void gicv3_its_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/gicv3_its.c wave=wave7 loc=2335
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::gicv3_its {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::gicv3_its
