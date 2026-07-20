export module pbsd.port.wave2.hbsd.src.usr_bin.diff.diff;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/diff/diff.c
// void diff_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/diff/diff.c wave=wave2 loc=698
export namespace pbsd::port::wave2::hbsd::src::usr_bin::diff::diff {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::diff::diff
