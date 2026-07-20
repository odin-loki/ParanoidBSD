export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.mktemp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/mktemp.c
// void mktemp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/mktemp.c wave=wave2 loc=197
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::mktemp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::mktemp
