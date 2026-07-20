export module pbsd.port.wave2.hbsd.src.usr_bin.head.head;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/head/head.c
// void head_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/head/head.c wave=wave2 loc=215
export namespace pbsd::port::wave2::hbsd::src::usr_bin::head::head {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::head::head
