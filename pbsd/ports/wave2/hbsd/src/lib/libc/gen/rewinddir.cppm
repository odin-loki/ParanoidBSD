export module pbsd.port.wave2.hbsd.src.lib.libc.gen.rewinddir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/rewinddir.c
// void rewinddir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/rewinddir.c wave=wave2 loc=59
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::rewinddir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::rewinddir
