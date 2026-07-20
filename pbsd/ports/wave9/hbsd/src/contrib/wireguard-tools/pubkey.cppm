export module pbsd.port.wave9.hbsd.src.contrib.wireguard_tools.pubkey;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wireguard-tools/pubkey.c
// void pubkey_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wireguard-tools/pubkey.c wave=wave9 loc=50
export namespace pbsd::port::wave9::hbsd::src::contrib::wireguard_tools::pubkey {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wireguard_tools::pubkey
