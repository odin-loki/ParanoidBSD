export module pbsd.port.wave2.hbsd.src.lib.libc.gen.inotify;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/inotify.c
// void inotify_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/inotify.c wave=wave2 loc=48
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::inotify {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::inotify
