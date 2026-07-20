export module pbsd.port.wave2.hbsd.src.lib.libc.gen.psignal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/psignal.c
// void psignal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/psignal.c wave=wave2 loc=63
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::psignal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::psignal
