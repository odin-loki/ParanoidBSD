export module pbsd.port.wave2.hbsd.src.usr_bin.tail.extern;

module;
// Header bridge — replace #include of hbsd/src/usr.bin/tail/extern.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/tail/extern.h wave=wave2 loc=77
export namespace pbsd::port::wave2::hbsd::src::usr_bin::tail::extern {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::tail::extern
