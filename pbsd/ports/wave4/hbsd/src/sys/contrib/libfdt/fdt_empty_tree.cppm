export module pbsd.port.wave4.hbsd.src.sys.contrib.libfdt.fdt_empty_tree;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libfdt/fdt_empty_tree.c
// void fdt_empty_tree_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libfdt/fdt_empty_tree.c wave=wave4 loc=83
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libfdt::fdt_empty_tree {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libfdt::fdt_empty_tree
