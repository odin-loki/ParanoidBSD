module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fmtmsg;

export import pbsd.core;

/// fmtmsg from hbsd/src/lib/libc/gen/fmtmsg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int fmtmsg_severity_mask(int classification) noexcept {
    return classification & 0x7;
}

} // namespace pbsd::userland::libc
