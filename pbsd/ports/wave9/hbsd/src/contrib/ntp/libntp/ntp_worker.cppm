export module pbsd.port.wave9.hbsd.src.contrib.ntp.libntp.ntp_worker;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/libntp/ntp_worker.c
// void ntp_worker_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/libntp/ntp_worker.c wave=wave9 loc=368
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::ntp_worker {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::libntp::ntp_worker
