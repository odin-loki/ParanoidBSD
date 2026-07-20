export module pbsd.port.wave5.hbsd.src.sys.dev.iavf.iavf_osdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/iavf/iavf_osdep.c
// void iavf_osdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/iavf/iavf_osdep.c wave=wave5 loc=404
export namespace pbsd::port::wave5::hbsd::src::sys::dev::iavf::iavf_osdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::iavf::iavf_osdep
