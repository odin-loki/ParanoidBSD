export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.nat_cmd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/nat_cmd.c
// void nat_cmd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/nat_cmd.c wave=wave2 loc=601
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::nat_cmd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::nat_cmd
