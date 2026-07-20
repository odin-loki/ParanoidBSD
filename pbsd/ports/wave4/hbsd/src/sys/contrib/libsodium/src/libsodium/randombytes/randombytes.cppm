export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.randombytes.randombytes;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/randombytes/randombytes.c
// void randombytes_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/randombytes/randombytes.c wave=wave4 loc=206
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::randombytes::randombytes {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::randombytes::randombytes
