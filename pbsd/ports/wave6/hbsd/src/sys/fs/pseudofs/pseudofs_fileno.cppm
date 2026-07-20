export module pbsd.port.wave6.hbsd.src.sys.fs.pseudofs.pseudofs_fileno;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/pseudofs/pseudofs_fileno.c
// void pseudofs_fileno_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/pseudofs/pseudofs_fileno.c wave=wave6 loc=157
export namespace pbsd::port::wave6::hbsd::src::sys::fs::pseudofs::pseudofs_fileno {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::pseudofs::pseudofs_fileno
