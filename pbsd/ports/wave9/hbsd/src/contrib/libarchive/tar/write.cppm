export module pbsd.port.wave9.hbsd.src.contrib.libarchive.tar.write;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libarchive/tar/write.c
// void write_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libarchive/tar/write.c wave=wave9 loc=1071
export namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::tar::write {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libarchive::tar::write
