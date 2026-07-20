export module pbsd.port.wave9.hbsd.src.crypto.heimdal.lib.roken.localtime_r;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/heimdal/lib/roken/localtime_r.c
// void localtime_r_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/heimdal/lib/roken/localtime_r.c wave=wave9 loc=60
export namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::roken::localtime_r {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::heimdal::lib::roken::localtime_r
