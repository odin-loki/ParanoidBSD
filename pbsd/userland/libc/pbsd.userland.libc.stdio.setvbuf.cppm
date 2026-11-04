module;

#include <cstddef>

export module pbsd.userland.libc.stdio.setvbuf;

import pbsd.userland.libc.stdio.file;

/// setvbuf from hbsd/src/lib/libc/stdio/setvbuf.c
export namespace pbsd::userland::libc::stdio {

enum class IoBufMode : int { Full = 0, Line = 1, NoBuf = 2 };

[[nodiscard]] inline int setvbuf(IoFile& fp, char* buf, IoBufMode mode, std::size_t size) noexcept {
    (void)mode;
    fp.bf = reinterpret_cast<unsigned char*>(buf);
    fp.bfsize = static_cast<int>(size);
    return 0;
}

} // namespace pbsd::userland::libc::stdio
