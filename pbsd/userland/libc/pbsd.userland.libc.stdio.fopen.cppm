module;

export module pbsd.userland.libc.stdio.fopen;

import pbsd.userland.libc.stdio.file;

/// fopen mode parse from hbsd/src/lib/libc/stdio/fopen.c (logic-only)
export namespace pbsd::userland::libc::stdio {

enum class OpenMode : unsigned char { Read, Write, Append, ReadWrite, ReadWritePlus };

[[nodiscard]] inline bool parse_fopen_mode(const char* mode, OpenMode& out) noexcept {
    if (mode == nullptr || mode[0] == '\0') {
        return false;
    }
    switch (mode[0]) {
    case 'r':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Read;
        return true;
    case 'w':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Write;
        return true;
    case 'a':
        out = (mode[1] == '+') ? OpenMode::ReadWritePlus : OpenMode::Append;
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline bool mode_allows_read(OpenMode m) noexcept {
    return m == OpenMode::Read || m == OpenMode::ReadWrite || m == OpenMode::ReadWritePlus;
}

[[nodiscard]] inline bool mode_allows_write(OpenMode m) noexcept {
    return m != OpenMode::Read;
}

} // namespace pbsd::userland::libc::stdio
