export module pbsd.port.wave4.hbsd.src.sys.contrib.ncsw.etc.error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/ncsw/etc/error.c
// void error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/ncsw/etc/error.c wave=wave4 loc=95
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::ncsw::etc::error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::ncsw::etc::error
