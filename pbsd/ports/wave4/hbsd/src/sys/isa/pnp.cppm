export module pbsd.port.wave4.hbsd.src.sys.isa.pnp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/isa/pnp.c
// void pnp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/isa/pnp.c wave=wave4 loc=767
export namespace pbsd::port::wave4::hbsd::src::sys::isa::pnp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::isa::pnp
