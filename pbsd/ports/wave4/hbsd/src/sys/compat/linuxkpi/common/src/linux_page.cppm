export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_page;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_page.c
// void linux_page_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_page.c wave=wave4 loc=797
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_page {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_page
