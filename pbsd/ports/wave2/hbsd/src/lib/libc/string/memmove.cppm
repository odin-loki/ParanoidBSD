export module pbsd.port.wave2.hbsd.src.lib.libc.string.memmove;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/memmove.c
// void memmove_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/memmove.c wave=wave2 loc=2
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memmove {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::memmove
