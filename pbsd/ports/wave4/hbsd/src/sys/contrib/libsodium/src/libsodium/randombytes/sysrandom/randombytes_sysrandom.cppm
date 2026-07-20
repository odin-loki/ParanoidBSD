export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.randombytes.sysrandom.randombytes_sysrandom;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c
// void randombytes_sysrandom_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c wave=wave4 loc=382
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::randombytes::sysrandom::randombytes_sysrandom {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::randombytes::sysrandom::randombytes_sysrandom
