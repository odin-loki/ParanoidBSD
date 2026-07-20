export module pbsd.port.wave2.hbsd.src.bin.ls.extern;

module;
// Header bridge — replace #include of hbsd/src/bin/ls/extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/ls/extern.h wave=wave2 loc=77
export namespace pbsd::port::wave2::hbsd::src::bin::ls::extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::ls::extern
