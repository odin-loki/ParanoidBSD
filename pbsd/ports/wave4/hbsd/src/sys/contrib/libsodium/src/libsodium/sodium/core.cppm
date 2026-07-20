export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.sodium.core;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/sodium/core.c
// void core_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/sodium/core.c wave=wave4 loc=231
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::sodium::core {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::sodium::core
