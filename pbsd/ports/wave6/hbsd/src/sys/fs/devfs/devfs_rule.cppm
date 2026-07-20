export module pbsd.port.wave6.hbsd.src.sys.fs.devfs.devfs_rule;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/fs/devfs/devfs_rule.c
// void devfs_rule_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/fs/devfs/devfs_rule.c wave=wave6 loc=822
export namespace pbsd::port::wave6::hbsd::src::sys::fs::devfs::devfs_rule {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::fs::devfs::devfs_rule
