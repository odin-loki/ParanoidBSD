export module pbsd.port.wave2.hbsd.src.bin.stty.extern;

module;
// Header bridge — replace #include of hbsd/src/bin/stty/extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/stty/extern.h wave=wave2 loc=42
export namespace pbsd::port::wave2::hbsd::src::bin::stty::extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::stty::extern
