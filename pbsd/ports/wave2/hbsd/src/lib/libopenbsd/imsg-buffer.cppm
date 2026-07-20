export module pbsd.port.wave2.hbsd.src.lib.libopenbsd.imsg_buffer;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libopenbsd/imsg-buffer.c
// void imsg-buffer_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libopenbsd/imsg-buffer.c wave=wave2 loc=307
export namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::imsg_buffer {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::imsg_buffer
