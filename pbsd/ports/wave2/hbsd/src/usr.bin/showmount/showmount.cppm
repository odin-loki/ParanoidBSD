export module pbsd.port.wave2.hbsd.src.usr_bin.showmount.showmount;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/showmount/showmount.c
// void showmount_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/showmount/showmount.c wave=wave2 loc=414
export namespace pbsd::port::wave2::hbsd::src::usr_bin::showmount::showmount {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::showmount::showmount
