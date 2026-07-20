export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.strlcat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/strlcat.c
// void strlcat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/strlcat.c wave=wave9 loc=63
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::strlcat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::strlcat
