module;

export module pbsd.userland.procstat.procstat_vm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_vm.c
export namespace pbsd::userland::usr_bin::procstat::procstat_vm {

[[nodiscard]] inline bool procstat_vm_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_vm
