export module pbsd.port.wave2.hbsd.src.bin.sh.shell;

module;
// Header bridge — replace #include of hbsd/src/bin/sh/shell.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/sh/shell.h wave=wave2 loc=74
export namespace pbsd::port::wave2::hbsd::src::bin::sh::shell {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::sh::shell
