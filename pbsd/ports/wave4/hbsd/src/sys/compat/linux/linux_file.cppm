export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_file;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_file.c
// void linux_file_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_file.c wave=wave4 loc=2055
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_file {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_file
