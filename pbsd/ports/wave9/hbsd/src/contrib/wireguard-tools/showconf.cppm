export module pbsd.port.wave9.hbsd.src.contrib.wireguard_tools.showconf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wireguard-tools/showconf.c
// void showconf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wireguard-tools/showconf.c wave=wave9 loc=103
export namespace pbsd::port::wave9::hbsd::src::contrib::wireguard_tools::showconf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wireguard_tools::showconf
