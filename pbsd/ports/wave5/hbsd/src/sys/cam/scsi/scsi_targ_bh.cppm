export module pbsd.port.wave5.hbsd.src.sys.cam.scsi.scsi_targ_bh;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cam/scsi/scsi_targ_bh.c
// void scsi_targ_bh_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cam/scsi/scsi_targ_bh.c wave=wave5 loc=765
export namespace pbsd::port::wave5::hbsd::src::sys::cam::scsi::scsi_targ_bh {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::cam::scsi::scsi_targ_bh
