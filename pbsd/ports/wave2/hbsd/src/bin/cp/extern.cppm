export module pbsd.port.wave2.hbsd.src.bin.cp.extern;

module;
// Header bridge — replace #include of hbsd/src/bin/cp/extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/cp/extern.h wave=wave2 loc=64
export namespace pbsd::port::wave2::hbsd::src::bin::cp::extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::cp::extern
