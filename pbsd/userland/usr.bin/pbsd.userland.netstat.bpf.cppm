module;

export module pbsd.userland.netstat.bpf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/bpf.c
export namespace pbsd::userland::usr_bin::netstat::bpf {

[[nodiscard]] inline bool bpf_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::bpf
