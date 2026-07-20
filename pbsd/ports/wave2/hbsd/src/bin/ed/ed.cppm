export module pbsd.port.wave2.hbsd.src.bin.ed.ed;

module;
// Header bridge — replace #include of hbsd/src/bin/ed/ed.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/ed/ed.h wave=wave2 loc=270
export namespace pbsd::port::wave2::hbsd::src::bin::ed::ed {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::ed::ed
