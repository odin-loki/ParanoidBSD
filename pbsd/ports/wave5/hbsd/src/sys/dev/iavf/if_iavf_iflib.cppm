export module pbsd.port.wave5.hbsd.src.sys.dev.iavf.if_iavf_iflib;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/iavf/if_iavf_iflib.c
// void if_iavf_iflib_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/iavf/if_iavf_iflib.c wave=wave5 loc=2151
export namespace pbsd::port::wave5::hbsd::src::sys::dev::iavf::if_iavf_iflib {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::iavf::if_iavf_iflib
