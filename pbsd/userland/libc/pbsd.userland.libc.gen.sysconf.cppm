module;
#include <cstdint>

export module pbsd.userland.libc.gen.sysconf;

export import pbsd.core;

/// sysconf scaffold from hbsd/src/lib/libc/gen/sysconf.c
export namespace pbsd::userland::libc {

enum class SysConf : int {
    ArgMax = 1,
    ChildMax = 2,
    ClkTck = 3,
    OpenMax = 4,
    PageSize = 5,
    NprocessorsOnln = 6,
};

[[nodiscard]] inline StatusOnly sysconf(SysConf name, long& out) noexcept {
    switch (name) {
    case SysConf::ArgMax:
        out = 262144;
        return status_ok();
    case SysConf::ChildMax:
        out = 1024;
        return status_ok();
    case SysConf::ClkTck:
        out = 100;
        return status_ok();
    case SysConf::OpenMax:
        out = 1024;
        return status_ok();
    case SysConf::PageSize:
        out = 4096;
        return status_ok();
    case SysConf::NprocessorsOnln:
        out = 1;
        return status_ok();
    }
    return status_err(Status::Invalid);
}

} // namespace pbsd::userland::libc
