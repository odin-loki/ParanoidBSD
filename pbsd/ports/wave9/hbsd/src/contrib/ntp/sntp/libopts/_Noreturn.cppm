export module pbsd.port.wave9.hbsd.src.contrib.ntp.sntp.libopts._noreturn;

module;
// Header bridge — replace #include of hbsd/src/contrib/ntp/sntp/libopts/_Noreturn.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/sntp/libopts/_Noreturn.h wave=wave9 loc=10
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libopts::_noreturn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::sntp::libopts::_noreturn
