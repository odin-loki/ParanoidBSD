export module pbsd.port.wave5.hbsd.src.sys.cam.scsi.scsi_all;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cam/scsi/scsi_all.c
// void scsi_all_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cam/scsi/scsi_all.c wave=wave5 loc=9429
export namespace pbsd::port::wave5::hbsd::src::sys::cam::scsi::scsi_all {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::cam::scsi::scsi_all
