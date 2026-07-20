export module pbsd.port.wave2.hbsd.src.lib.libc.string.mempcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/mempcpy.c
// void mempcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/mempcpy.c wave=wave2 loc=39
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::mempcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::mempcpy
