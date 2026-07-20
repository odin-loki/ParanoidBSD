export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.sodium.version;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/sodium/version.c
// void version_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/sodium/version.c wave=wave4 loc=30
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::sodium::version {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::sodium::version
