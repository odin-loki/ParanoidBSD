export module pbsd.port.wave2.hbsd.src.usr_bin.du.du;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/du/du.c
// void du_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/du/du.c wave=wave2 loc=579
export namespace pbsd::port::wave2::hbsd::src::usr_bin::du::du {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::du::du
