module;

#include <cstddef>
#include <cstring>

export module pbsd.userland.libc.regex.regerror;

/// regerror from hbsd/src/lib/libc/regex/regerror.c
export namespace pbsd::userland::libc {

inline constexpr int kRegNomatch = 1;
inline constexpr int kRegBadpat = 2;
inline constexpr int kRegEcollate = 3;
inline constexpr int kRegEctype = 4;
inline constexpr int kRegEescape = 5;
inline constexpr int kRegEsubreg = 6;
inline constexpr int kRegEbrack = 7;
inline constexpr int kRegEparen = 8;
inline constexpr int kRegEbrace = 9;
inline constexpr int kRegBadbr = 10;
inline constexpr int kRegErange = 11;
inline constexpr int kRegEspace = 12;
inline constexpr int kRegBadrpt = 13;
inline constexpr int kRegEmpty = 14;
inline constexpr int kRegAssert = 15;
inline constexpr int kRegInvarg = 16;
inline constexpr int kRegIllseq = 17;

struct RegErrorEntry {
    int code;
    const char* explain;
};

inline constexpr RegErrorEntry kRegErrors[] = {
    {kRegNomatch, "regexec() failed to match"},
    {kRegBadpat, "invalid regular expression"},
    {kRegEcollate, "invalid collating element"},
    {kRegEctype, "invalid character class"},
    {kRegEescape, "trailing backslash (\\)"},
    {kRegEsubreg, "invalid backreference number"},
    {kRegEbrack, "brackets ([ ]) not balanced"},
    {kRegEparen, "parentheses not balanced"},
    {kRegEbrace, "braces not balanced"},
    {kRegBadbr, "invalid repetition count(s)"},
    {kRegErange, "invalid character range"},
    {kRegEspace, "out of memory"},
    {kRegBadrpt, "repetition-operator operand invalid"},
    {kRegEmpty, "empty (sub)expression"},
    {kRegAssert, "\"can't happen\" -- you found a bug"},
    {kRegInvarg, "invalid argument to regex routine"},
    {kRegIllseq, "illegal byte sequence"},
    {0, "*** unknown regexp error code ***"},
};

[[nodiscard]] inline const char* regerror_message(int errcode) noexcept {
    for (const auto& entry : kRegErrors) {
        if (entry.code == errcode) {
            return entry.explain;
        }
    }
    return kRegErrors[sizeof(kRegErrors) / sizeof(kRegErrors[0]) - 1].explain;
}

[[nodiscard]] inline std::size_t regerror(int errcode, char* errbuf, std::size_t errbuf_size) noexcept {
    const char* msg = regerror_message(errcode);
    const std::size_t len = std::strlen(msg);
    if (errbuf != nullptr && errbuf_size > 0) {
        const std::size_t copy = (len < errbuf_size - 1) ? len : errbuf_size - 1;
        std::memcpy(errbuf, msg, copy);
        errbuf[copy] = '\0';
    }
    return len + 1;
}

} // namespace pbsd::userland::libc
