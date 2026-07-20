export module pbsd.port.wave2.hbsd.src.libexec.bootpd.bootpgw.bootpgw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/bootpd/bootpgw/bootpgw.c
// void bootpgw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/bootpd/bootpgw/bootpgw.c wave=wave2 loc=674
export namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::bootpgw::bootpgw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::bootpgw::bootpgw
