export module pbsd.port.wave2.hbsd.src.usr_bin.mkimg.qcow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/mkimg/qcow.c
// void qcow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/mkimg/qcow.c wave=wave2 loc=367
export namespace pbsd::port::wave2::hbsd::src::usr_bin::mkimg::qcow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::mkimg::qcow
