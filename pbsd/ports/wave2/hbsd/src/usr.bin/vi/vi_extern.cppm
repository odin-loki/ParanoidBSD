export module pbsd.port.wave2.hbsd.src.usr_bin.vi.vi_extern;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/vi/vi_extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/vi/vi_extern.h wave=wave2 loc=145
export namespace pbsd::port::wave2::hbsd::src::usr_bin::vi::vi_extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::vi::vi_extern
