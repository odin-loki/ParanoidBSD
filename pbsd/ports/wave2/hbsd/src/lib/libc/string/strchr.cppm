export module pbsd.port.wave2.hbsd.src.lib.libc.string.strchr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/strchr.c
// void strchr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/strchr.c wave=wave2 loc=36
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strchr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::strchr
