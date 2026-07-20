export module pbsd.port.wave9.hbsd.src.secure.lib.libssl.dummy_abi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/secure/lib/libssl/dummy_abi.c
// void dummy_abi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/secure/lib/libssl/dummy_abi.c wave=wave9 loc=0
export namespace pbsd::port::wave9::hbsd::src::secure::lib::libssl::dummy_abi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::secure::lib::libssl::dummy_abi
