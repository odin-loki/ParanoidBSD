export module pbsd.port.wave2.hbsd.src.lib.libipsec.pfkey_dump;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libipsec/pfkey_dump.c
// void pfkey_dump_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libipsec/pfkey_dump.c wave=wave2 loc=697
export namespace pbsd::port::wave2::hbsd::src::lib::libipsec::pfkey_dump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libipsec::pfkey_dump
