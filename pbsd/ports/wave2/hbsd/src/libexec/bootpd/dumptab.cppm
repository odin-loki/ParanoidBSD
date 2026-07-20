export module pbsd.port.wave2.hbsd.src.libexec.bootpd.dumptab;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/bootpd/dumptab.c
// void dumptab_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/bootpd/dumptab.c wave=wave2 loc=361
export namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::dumptab {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::dumptab
