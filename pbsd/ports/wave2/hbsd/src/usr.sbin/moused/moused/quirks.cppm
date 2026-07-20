export module pbsd.port.wave2.hbsd.src.usr_sbin.moused.moused.quirks;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/moused/moused/quirks.c
// void quirks_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/moused/moused/quirks.c wave=wave2 loc=2033
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::moused::moused::quirks {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::moused::moused::quirks
