module;

#include <cstddef>
#include <cstring>

export module pbsd.userland.util.fparseln;

/// fparseln(3) flags + line parsing from hbsd/src/lib/libutil/fparseln.c
export namespace pbsd::userland::util {

inline constexpr int FPARSELN_UNESCESC  = 0x01;
inline constexpr int FPARSELN_UNESCCONT = 0x02;
inline constexpr int FPARSELN_UNESCCOMM = 0x04;
inline constexpr int FPARSELN_UNESCREST = 0x08;
inline constexpr int FPARSELN_UNESCALL  = 0x0f;

struct ParseChars {
    char escape{'\\'};
    char continuation{'\\'};
    char comment{'#'};
};

[[nodiscard]] inline bool is_escaped(const char* start, const char* p,
                                     char esc) noexcept {
    if (esc == '\0') {
        return false;
    }
    unsigned ne = 0;
    for (const char* cp = p; --cp >= start && *cp == esc; ++ne) {
    }
    return (ne & 1u) != 0u;
}

/// Strip trailing newline and detect continuation marker.
[[nodiscard]] inline bool strip_line_end(char* line, unsigned& len,
                                         const ParseChars& pc) noexcept {
    if (len == 0) {
        return false;
    }
    if (pc.comment != '\0') {
        for (unsigned i = 0; i < len; ++i) {
            if (line[i] == pc.comment &&
                !is_escaped(line, line + i, pc.escape)) {
                len = i;
                break;
            }
        }
    }
    if (len > 0 && line[len - 1] == '\n') {
        --len;
        line[len] = '\0';
    }
    if (len > 0 && pc.continuation != '\0') {
        if (line[len - 1] == pc.continuation &&
            !is_escaped(line, line + len - 1, pc.escape)) {
            --len;
            line[len] = '\0';
            return true;
        }
    }
    return false;
}

/// Apply FPARSELN_UNESC* rules to an in-place buffer; returns new length.
[[nodiscard]] inline unsigned unescape_line(char* buf, unsigned len,
                                            int flags,
                                            const ParseChars& pc) noexcept {
    if (buf == nullptr || len == 0) {
        return len;
    }
    if ((flags & FPARSELN_UNESCALL) == 0 || pc.escape == '\0') {
        return len;
    }
    if (std::strchr(buf, pc.escape) == nullptr) {
        return len;
    }

    char* ptr = buf;
    char* cp = buf;
    while (cp[0] != '\0') {
        while (cp[0] != '\0' && cp[0] != pc.escape) {
            *ptr++ = *cp++;
        }
        if (cp[0] == '\0' || cp[1] == '\0') {
            break;
        }

        int skipesc = 0;
        if (cp[1] == pc.comment) {
            skipesc += (flags & FPARSELN_UNESCCOMM);
        }
        if (cp[1] == pc.continuation) {
            skipesc += (flags & FPARSELN_UNESCCONT);
        }
        if (cp[1] == pc.escape) {
            skipesc += (flags & FPARSELN_UNESCESC);
        }
        if (cp[1] != pc.comment && cp[1] != pc.continuation &&
            cp[1] != pc.escape) {
            skipesc = (flags & FPARSELN_UNESCREST);
        }

        if (skipesc != 0) {
            ++cp;
        } else {
            *ptr++ = *cp++;
        }
        *ptr++ = *cp++;
    }
    *ptr = '\0';
    return static_cast<unsigned>(ptr - buf);
}

} // namespace pbsd::userland::util
