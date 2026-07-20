export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.tmpnam;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/tmpnam.c
// void tmpnam_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/tmpnam.c wave=wave2 loc=57
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::tmpnam {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::tmpnam
