export module pbsd.port.wave2.hbsd.src.lib.libipsec.pfkey;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libipsec/pfkey.c
// void pfkey_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libipsec/pfkey.c wave=wave2 loc=2066
export namespace pbsd::port::wave2::hbsd::src::lib::libipsec::pfkey {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libipsec::pfkey
