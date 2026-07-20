export module pbsd.port.wave2.hbsd.src.libexec.bootpd.tzone;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/bootpd/tzone.c
// void tzone_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/bootpd/tzone.c wave=wave2 loc=46
export namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::tzone {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::bootpd::tzone
