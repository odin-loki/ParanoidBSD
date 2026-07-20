export module pbsd.port.wave2.hbsd.src.usr_bin.sdiotool.linux_compat;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/sdiotool/linux_compat.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/sdiotool/linux_compat.h wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::usr_bin::sdiotool::linux_compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::sdiotool::linux_compat
