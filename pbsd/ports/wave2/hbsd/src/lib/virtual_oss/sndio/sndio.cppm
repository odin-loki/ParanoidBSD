export module pbsd.port.wave2.hbsd.src.lib.virtual_oss.sndio.sndio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/virtual_oss/sndio/sndio.c
// void sndio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/virtual_oss/sndio/sndio.c wave=wave2 loc=203
export namespace pbsd::port::wave2::hbsd::src::lib::virtual_oss::sndio::sndio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::virtual_oss::sndio::sndio
