export module pbsd.port.wave2.hbsd.src.lib.libc.string.ffs;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/string/ffs.c
// void ffs_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/string/ffs.c wave=wave2 loc=45
export namespace pbsd::port::wave2::hbsd::src::lib::libc::string::ffs {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::string::ffs
