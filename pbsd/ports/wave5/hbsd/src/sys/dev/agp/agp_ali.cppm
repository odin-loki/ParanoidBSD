export module pbsd.port.wave5.hbsd.src.sys.dev.agp.agp_ali;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/agp/agp_ali.c
// void agp_ali_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/agp/agp_ali.c wave=wave5 loc=267
export namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_ali {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::agp::agp_ali
