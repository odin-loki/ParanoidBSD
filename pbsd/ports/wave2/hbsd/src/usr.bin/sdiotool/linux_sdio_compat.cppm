export module pbsd.port.wave2.hbsd.src.usr_bin.sdiotool.linux_sdio_compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/sdiotool/linux_sdio_compat.c
// void linux_sdio_compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/sdiotool/linux_sdio_compat.c wave=wave2 loc=100
export namespace pbsd::port::wave2::hbsd::src::usr_bin::sdiotool::linux_sdio_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::sdiotool::linux_sdio_compat
