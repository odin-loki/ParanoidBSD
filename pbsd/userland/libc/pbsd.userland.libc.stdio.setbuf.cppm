module;

export module pbsd.userland.libc.stdio.setbuf;

import pbsd.userland.libc.stdio.file;

/// setbuf/setbuffer from hbsd/src/lib/libc/stdio/setbuf.c
export namespace pbsd::userland::libc::stdio {

inline void setbuf(IoFile& fp, char* buf) noexcept {
    fp.bf = reinterpret_cast<unsigned char*>(buf);
    fp.bfsize = buf != nullptr ? 0 : 0;
}

[[nodiscard]] inline int setbuffer(IoFile& fp, char* buf, int size) noexcept {
    fp.bf = reinterpret_cast<unsigned char*>(buf);
    fp.bfsize = size;
    return 0;
}

} // namespace pbsd::userland::libc::stdio
