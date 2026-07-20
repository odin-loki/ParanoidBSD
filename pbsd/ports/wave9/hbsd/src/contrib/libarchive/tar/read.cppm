export module pbsd.port.wave9.hbsd.src.contrib.libarchive.tar.read;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libarchive/tar/read.c
// void read_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libarchive/tar/read.c wave=wave9 loc=401
export namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::tar::read {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::tar::read
