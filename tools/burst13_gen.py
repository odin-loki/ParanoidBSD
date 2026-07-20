#!/usr/bin/env python3
"""Burst 13: generate thin PBSD userland .cppm modules."""
from pathlib import Path

root = Path(__file__).resolve().parent.parent / "pbsd" / "userland"
NUL = "'\\0'"

libc_modules = [
("gen.usleep", f'''module;
#include <cstdint>

export module pbsd.userland.libc.gen.usleep;

export import pbsd.core;

/// usleep from hbsd/src/lib/libc/gen/usleep.c
export namespace pbsd::userland::libc {{

struct Timespec {{
    std::int64_t tv_sec{{0}};
    std::int64_t tv_nsec{{0}};
}};

[[nodiscard]] inline Timespec usleep_to_timespec(unsigned usec) noexcept {{
    Timespec ts{{}};
    ts.tv_sec = static_cast<std::int64_t>(usec / 1000000u);
    ts.tv_nsec = static_cast<std::int64_t>((usec % 1000000u) * 1000u);
    return ts;
}}

}} // namespace pbsd::userland::libc
'''),
("gen.alarm", '''module;
#include <cstdint>

export module pbsd.userland.libc.gen.alarm;

export import pbsd.core;

/// alarm from hbsd/src/lib/libc/gen/alarm.c
export namespace pbsd::userland::libc {

struct Itimerval {
    std::int64_t interval_sec{0};
    std::int64_t interval_usec{0};
    std::int64_t value_sec{0};
    std::int64_t value_usec{0};
};

[[nodiscard]] inline Itimerval alarm_itimerval(unsigned secs) noexcept {
    Itimerval it{};
    it.value_sec = static_cast<std::int64_t>(secs);
    return it;
}

[[nodiscard]] inline unsigned alarm_remaining_seconds(const Itimerval& prev) noexcept {
    unsigned rem = static_cast<unsigned>(prev.value_sec);
    if (prev.value_usec != 0) {
        ++rem;
    }
    return rem;
}

} // namespace pbsd::userland::libc
'''),
("gen.pause", '''module;

export module pbsd.userland.libc.gen.pause;

export import pbsd.core;

/// pause from hbsd/src/lib/libc/gen/pause.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool pause_until_signal() noexcept { return true; }

} // namespace pbsd::userland::libc
'''),
("gen.basename", f'''module;
#include <cstddef>

export module pbsd.userland.libc.gen.basename;

export import pbsd.core;

/// basename from hbsd/src/lib/libc/gen/basename.c
export namespace pbsd::userland::libc {{

[[nodiscard]] inline const char* basename_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return ".";
    }}
    const char* base = path;
    for (const char* p = path; *p != {NUL}; ++p) {{
        if (*p == '/' || *p == '\\\\') {{
            base = p + 1;
        }}
    }}
    return base[0] == {NUL} ? "." : base;
}}

}} // namespace pbsd::userland::libc
'''),
("gen.dirname", f'''module;
#include <cstddef>

export module pbsd.userland.libc.gen.dirname;

export import pbsd.core;

/// dirname from hbsd/src/lib/libc/gen/dirname.c
export namespace pbsd::userland::libc {{

[[nodiscard]] inline std::size_t dirname_length(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return 1;
    }}
    std::size_t len = 0;
    while (path[len] != {NUL}) {{
        ++len;
    }}
    while (len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\\\')) {{
        --len;
    }}
    while (len > 0 && path[len - 1] != '/' && path[len - 1] != '\\\\') {{
        --len;
    }}
    return len == 0 ? 1 : len;
}}

}} // namespace pbsd::userland::libc
'''),
("gen.sysconf", '''module;
#include <cstdint>

export module pbsd.userland.libc.gen.sysconf;

export import pbsd.core;

/// sysconf from hbsd/src/lib/libc/gen/sysconf.c
export namespace pbsd::userland::libc {

inline constexpr int kScPagesize = 2;
inline constexpr int kScNprocessorsOnln = 58;

[[nodiscard]] inline Result<std::int64_t> sysconf_pagesize(std::int64_t page_size) noexcept {
    if (page_size <= 0) {
        return result_err<std::int64_t>(Status::Invalid);
    }
    return result_ok(page_size);
}

} // namespace pbsd::userland::libc
'''),
("gen.ualarm", '''module;
#include <cstdint>

export module pbsd.userland.libc.gen.ualarm;

export import pbsd.core;

/// ualarm from hbsd/src/lib/libc/gen/ualarm.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned ualarm_total_usecs(unsigned usecs, unsigned interval) noexcept {
    if (interval == 0) {
        return usecs;
    }
    return usecs + interval;
}

} // namespace pbsd::userland::libc
'''),
("gen.times", '''module;
#include <cstdint>

export module pbsd.userland.libc.gen.times;

export import pbsd.core;

/// times from hbsd/src/lib/libc/gen/times.c
export namespace pbsd::userland::libc {

struct Tms {
    std::int64_t tms_utime{0};
    std::int64_t tms_stime{0};
    std::int64_t tms_cutime{0};
    std::int64_t tms_cstime{0};
};

[[nodiscard]] inline Tms times_zero() noexcept { return Tms{}; }

} // namespace pbsd::userland::libc
'''),
("gen.closedir", '''module;

export module pbsd.userland.libc.gen.closedir;

export import pbsd.core;

/// closedir from hbsd/src/lib/libc/gen/closedir.c
export namespace pbsd::userland::libc {

struct DirHandle { int fd{-1}; };

[[nodiscard]] inline Status closedir_handle(DirHandle& d) noexcept {
    if (d.fd < 0) {
        return Status::Invalid;
    }
    d.fd = -1;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
'''),
("gen.readdir", f'''module;
#include <cstddef>

export module pbsd.userland.libc.gen.readdir;

export import pbsd.core;

/// readdir from hbsd/src/lib/libc/gen/readdir.c
export namespace pbsd::userland::libc {{

struct Dirent {{ char d_name[256]{{}}; std::size_t d_namlen{{0}}; }};

[[nodiscard]] inline bool readdir_end(const Dirent* ent) noexcept {{
    return ent == nullptr || ent->d_name[0] == {NUL};
}}

}} // namespace pbsd::userland::libc
'''),
("gen.rewinddir", '''module;

export module pbsd.userland.libc.gen.rewinddir;

export import pbsd.core;

/// rewinddir from hbsd/src/lib/libc/gen/rewinddir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

inline void rewinddir_handle(DirHandle& d) noexcept { d.tell = 0; }

} // namespace pbsd::userland::libc
'''),
("gen.telldir", '''module;

export module pbsd.userland.libc.gen.telldir;

export import pbsd.core;

/// telldir from hbsd/src/lib/libc/gen/telldir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

[[nodiscard]] inline long telldir_pos(const DirHandle& d) noexcept { return d.tell; }

} // namespace pbsd::userland::libc
'''),
("gen.seekdir", '''module;

export module pbsd.userland.libc.gen.seekdir;

export import pbsd.core;

/// seekdir from hbsd/src/lib/libc/gen/seekdir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

inline void seekdir_pos(DirHandle& d, long pos) noexcept { d.tell = pos; }

} // namespace pbsd::userland::libc
'''),
("gen.getentropy", '''module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getentropy;

export import pbsd.core;

/// getentropy from hbsd/src/lib/libc/gen/getentropy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getentropy_validate(std::size_t len) noexcept {
    if (len == 0 || len > 256) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline void getentropy_fill(std::uint8_t* buf, std::size_t len,
                                            std::uint8_t seed) noexcept {
    for (std::size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<std::uint8_t>(seed + static_cast<std::uint8_t>(i));
    }
}

} // namespace pbsd::userland::libc
'''),
("gen.nice", '''module;

export module pbsd.userland.libc.gen.nice;

export import pbsd.core;

/// nice from hbsd/src/lib/libc/gen/nice.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int nice_increment(int incr) noexcept {
    if (incr < -40) {
        return -40;
    }
    if (incr > 40) {
        return 40;
    }
    return incr;
}

} // namespace pbsd::userland::libc
'''),
("gen.arc4random_uniform", '''module;
#include <cstdint>

export module pbsd.userland.libc.gen.arc4random_uniform;

/// arc4random_uniform from hbsd/src/lib/libc/gen/arc4random_uniform.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t arc4random_uniform(std::uint32_t upper_bound,
                                                     std::uint32_t raw) noexcept {
    if (upper_bound < 2) {
        return 0;
    }
    const std::uint32_t min = static_cast<std::uint32_t>(-upper_bound) % upper_bound;
    std::uint32_t r = raw;
    while (r < min) {
        r = raw;
    }
    return r % upper_bound;
}

} // namespace pbsd::userland::libc
'''),
("gen.ttyname", '''module;

export module pbsd.userland.libc.gen.ttyname;

export import pbsd.core;

/// ttyname from hbsd/src/lib/libc/gen/ttyname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<const char*> ttyname_for_fd(int fd) noexcept {
    if (fd < 0) {
        return result_err<const char*>(Status::Invalid);
    }
    return result_ok("/dev/tty");
}

} // namespace pbsd::userland::libc
'''),
("gen.setmode", '''module;

export module pbsd.userland.libc.gen.setmode;

export import pbsd.core;

/// setmode from hbsd/src/lib/libc/gen/setmode.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool setmode_is_operator(char c) noexcept {
    return c == '+' || c == '-' || c == ' ';
}

} // namespace pbsd::userland::libc
'''),
("gen.siglist", '''module;

export module pbsd.userland.libc.gen.siglist;

export import pbsd.core;

/// siglist from hbsd/src/lib/libc/gen/siglist.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* siglist_name(int sig) noexcept {
    switch (sig) {
    case 1: return "SIGHUP";
    case 2: return "SIGINT";
    case 9: return "SIGKILL";
    case 15: return "SIGTERM";
    default: return "unknown signal";
    }
}

} // namespace pbsd::userland::libc
'''),
("gen.syslog", '''module;

export module pbsd.userland.libc.gen.syslog;

export import pbsd.core;

/// syslog from hbsd/src/lib/libc/gen/syslog.c
export namespace pbsd::userland::libc {

enum class SyslogPriority : int { Emerg = 0, Alert, Crit, Err, Warning, Notice, Info, Debug };

[[nodiscard]] inline int syslog_priority_clamp(int pri) noexcept {
    if (pri < 0) {
        return 0;
    }
    if (pri > 7) {
        return 7;
    }
    return pri;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.atoi", '''module;

export module pbsd.userland.libc.stdlib.atoi;

export import pbsd.core;

/// atoi from hbsd/src/lib/libc/stdlib/atoi.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int atoi_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0;
    }
    int sign = 1;
    while (*nptr == ' ' || *nptr == '\\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    int acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.atol", '''module;

export module pbsd.userland.libc.stdlib.atol;

export import pbsd.core;

/// atol from hbsd/src/lib/libc/stdlib/atol.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long atol_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0L;
    }
    long sign = 1;
    while (*nptr == ' ' || *nptr == '\\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    long acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.atoll", '''module;

export module pbsd.userland.libc.stdlib.atoll;

export import pbsd.core;

/// atoll from hbsd/src/lib/libc/stdlib/atoll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long atoll_digits(const char* nptr) noexcept {
    if (nptr == nullptr) {
        return 0LL;
    }
    long long sign = 1;
    while (*nptr == ' ' || *nptr == '\\t') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    long long acc = 0;
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.qsort", '''module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.qsort;

export import pbsd.core;

/// qsort from hbsd/src/lib/libc/stdlib/qsort.c
export namespace pbsd::userland::libc {

using QsortCompare = int (*)(const void*, const void*) noexcept;

inline void qsort_swap(unsigned char* a, unsigned char* b, std::size_t size) noexcept {
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned char t = a[i];
        a[i] = b[i];
        b[i] = t;
    }
}

} // namespace pbsd::userland::libc
'''),
("stdlib.realpath", '''module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.realpath;

export import pbsd.core;

/// realpath from hbsd/src/lib/libc/stdlib/realpath.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status realpath_validate(const char* path, char* resolved,
                                              std::size_t len) noexcept {
    if (path == nullptr || resolved == nullptr || len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.reallocarray", '''module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.reallocarray;

export import pbsd.core;

/// reallocarray from hbsd/src/lib/libc/stdlib/reallocarray.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> reallocarray_size(std::size_t nmemb,
                                                           std::size_t size) noexcept {
    if (nmemb != 0 && size > (~std::size_t{0}) / nmemb) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(nmemb * size);
}

} // namespace pbsd::userland::libc
'''),
("stdlib.labs", '''module;

export module pbsd.userland.libc.stdlib.labs;

export import pbsd.core;

/// labs from hbsd/src/lib/libc/stdlib/labs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long labs_val(long j) noexcept { return j < 0 ? -j : j; }

} // namespace pbsd::userland::libc
'''),
("stdlib.ldiv", '''module;

export module pbsd.userland.libc.stdlib.ldiv;

export import pbsd.core;

/// ldiv from hbsd/src/lib/libc/stdlib/ldiv.c
export namespace pbsd::userland::libc {

struct LdivT { long quot{0}; long rem{0}; };

[[nodiscard]] inline LdivT ldiv_vals(long numer, long denom) noexcept {
    LdivT r{};
    if (denom == 0) {
        return r;
    }
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
'''),
("stdlib.abort", '''module;

export module pbsd.userland.libc.stdlib.abort;

export import pbsd.core;

/// abort from hbsd/src/lib/libc/stdlib/abort.c
export namespace pbsd::userland::libc {

[[noreturn]] inline void abort_trap() noexcept { for (;;) {} }

} // namespace pbsd::userland::libc
'''),
("stdio.feof", '''module;

export module pbsd.userland.libc.stdio.feof;

export import pbsd.core;

/// feof from hbsd/src/lib/libc/stdio/feof.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

[[nodiscard]] inline int feof_flag(const FileFlags& f) noexcept { return f.eof ? 1 : 0; }

} // namespace pbsd::userland::libc
'''),
("stdio.ferror", '''module;

export module pbsd.userland.libc.stdio.ferror;

export import pbsd.core;

/// ferror from hbsd/src/lib/libc/stdio/ferror.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

[[nodiscard]] inline int ferror_flag(const FileFlags& f) noexcept { return f.error ? 1 : 0; }

} // namespace pbsd::userland::libc
'''),
("stdio.ftell", '''module;

export module pbsd.userland.libc.stdio.ftell;

export import pbsd.core;

/// ftell from hbsd/src/lib/libc/stdio/ftell.c
export namespace pbsd::userland::libc {

struct FilePos { long offset{0}; };

[[nodiscard]] inline long ftell_pos(const FilePos& f) noexcept { return f.offset; }

} // namespace pbsd::userland::libc
'''),
("stdio.rewind", '''module;

export module pbsd.userland.libc.stdio.rewind;

export import pbsd.core;

/// rewind from hbsd/src/lib/libc/stdio/rewind.c
export namespace pbsd::userland::libc {

struct FilePos { long offset{0}; bool error{false}; };

inline void rewind_pos(FilePos& f) noexcept {
    f.offset = 0;
    f.error = false;
}

} // namespace pbsd::userland::libc
'''),
("stdio.getline", f'''module;
#include <cstddef>

export module pbsd.userland.libc.stdio.getline;

export import pbsd.core;

/// getline from hbsd/src/lib/libc/stdio/getline.c
export namespace pbsd::userland::libc {{

[[nodiscard]] inline Result<std::size_t> getline_need(std::size_t len, char delim) noexcept {{
    if (delim == {NUL}) {{
        return result_err<std::size_t>(Status::Invalid);
    }}
    return result_ok(len + 1);
}}

}} // namespace pbsd::userland::libc
'''),
("stdio.clrerr", '''module;

export module pbsd.userland.libc.stdio.clrerr;

export import pbsd.core;

/// clearerr from hbsd/src/lib/libc/stdio/clrerr.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

inline void clearerr_flags(FileFlags& f) noexcept {
    f.eof = false;
    f.error = false;
}

} // namespace pbsd::userland::libc
'''),
("net.base64", '''module;
#include <cstddef>

export module pbsd.userland.libc.net.base64;

export import pbsd.core;

/// base64 from hbsd/src/lib/libc/net/base64.c
export namespace pbsd::userland::libc {

inline constexpr char kBase64Pad = '=';

[[nodiscard]] inline std::size_t base64_encoded_len(std::size_t raw_len) noexcept {
    return ((raw_len + 2) / 3) * 4;
}

[[nodiscard]] inline bool base64_valid_char(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
           c == '+' || c == '/' || c == kBase64Pad;
}

} // namespace pbsd::userland::libc
'''),
("net.gai_strerror", '''module;

export module pbsd.userland.libc.net.gai_strerror;

export import pbsd.core;

/// gai_strerror from hbsd/src/lib/libc/net/gai_strerror.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* gai_strerror_msg(int ecode) noexcept {
    switch (ecode) {
    case 1: return "Temporary failure in name resolution";
    case 2: return "Non-recoverable failure in name resolution";
    case 3: return "Invalid flags";
    default: return "Unknown error";
    }
}

} // namespace pbsd::userland::libc
'''),
("net.if_nametoindex", f'''module;

export module pbsd.userland.libc.net.if_nametoindex;

export import pbsd.core;

/// if_nametoindex from hbsd/src/lib/libc/net/if_nametoindex.c
export namespace pbsd::userland::libc {{

[[nodiscard]] inline Result<unsigned> if_nametoindex_name(const char* ifname) noexcept {{
    if (ifname == nullptr || ifname[0] == {NUL}) {{
        return result_err<unsigned>(Status::Invalid);
    }}
    return result_ok(1u);
}}

}} // namespace pbsd::userland::libc
'''),
("sys.open", '''module;

export module pbsd.userland.libc.sys.open;

export import pbsd.core;

/// open from hbsd/src/lib/libc/sys/open.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status open_validate_path(const char* path) noexcept {
    if (path == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
'''),
("sys.read", '''module;
#include <cstddef>

export module pbsd.userland.libc.sys.read;

export import pbsd.core;

/// read from hbsd/src/lib/libc/sys/read.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> read_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
'''),
("sys.write", '''module;
#include <cstddef>

export module pbsd.userland.libc.sys.write;

export import pbsd.core;

/// write from hbsd/src/lib/libc/sys/write.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> write_count(std::size_t nbyte) noexcept {
    return result_ok(nbyte);
}

} // namespace pbsd::userland::libc
'''),
("sys.close", '''module;

export module pbsd.userland.libc.sys.close;

export import pbsd.core;

/// close from hbsd/src/lib/libc/sys/close.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status close_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
'''),
("sys.fork", '''module;

export module pbsd.userland.libc.sys.fork;

export import pbsd.core;

/// fork from hbsd/src/lib/libc/sys/fork.c
export namespace pbsd::userland::libc {

enum class ForkRole : int { Parent = 0, Child = 1 };

[[nodiscard]] inline ForkRole fork_role(int pid) noexcept {
    return pid == 0 ? ForkRole::Child : ForkRole::Parent;
}

} // namespace pbsd::userland::libc
'''),
("locale.rpmatch", '''module;

export module pbsd.userland.libc.locale.rpmatch;

export import pbsd.core;

/// rpmatch from hbsd/src/lib/libc/locale/rpmatch.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<int> rpmatch_response(const char* resp) noexcept {
    if (resp == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    switch (resp[0]) {
    case 'y':
    case 'Y':
        return result_ok(1);
    case 'n':
    case 'N':
        return result_ok(0);
    default:
        return result_err<int>(Status::Invalid);
    }
}

} // namespace pbsd::userland::libc
'''),
("locale.tolower", '''module;

export module pbsd.userland.libc.locale.tolower;

export import pbsd.core;

/// tolower from hbsd/src/lib/libc/locale/tolower.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int tolower_ascii(int c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

} // namespace pbsd::userland::libc
'''),
("locale.toupper", '''module;

export module pbsd.userland.libc.locale.toupper;

export import pbsd.core;

/// toupper from hbsd/src/lib/libc/locale/toupper.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int toupper_ascii(int c) noexcept {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

} // namespace pbsd::userland::libc
'''),
]

libthr_modules = {
"self": '''module;

export module pbsd.userland.libthr.self;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_self from hbsd/src/lib/libthr/thread/thr_self.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline ThreadId pthread_self_id() noexcept { return thread_self(); }

} // namespace pbsd::userland::libthr
''',
"create": '''module;

export module pbsd.userland.libthr.create;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_create from hbsd/src/lib/libthr/thread/thr_create.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pthread_create_thread(ThreadId& out, ThreadAttr attr,
                                                  void (*start)(void*), void* arg) noexcept {
    return thread_create(out, attr, start, arg);
}

} // namespace pbsd::userland::libthr
''',
"fork": '''module;

export module pbsd.userland.libthr.fork;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_atfork from hbsd/src/lib/libthr/thread/thr_fork.c
export namespace pbsd::userland::libthr {

using AtForkHandler = void (*)() noexcept;

struct AtForkHandlers {
    AtForkHandler prepare{nullptr};
    AtForkHandler parent{nullptr};
    AtForkHandler child{nullptr};
};

[[nodiscard]] inline Status atfork_register(AtForkHandlers& h, AtForkHandler prepare,
                                            AtForkHandler parent,
                                            AtForkHandler child) noexcept {
    h.prepare = prepare;
    h.parent = parent;
    h.child = child;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"getthreadid_np": '''module;
#include <cstdint>

export module pbsd.userland.libthr.getthreadid_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_getthreadid_np from hbsd/src/lib/libthr/thread/thr_getthreadid_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline std::uint64_t getthreadid_np(ThreadId tid) noexcept {
    return static_cast<std::uint64_t>(tid);
}

} // namespace pbsd::userland::libthr
''',
"getschedparam": '''module;

export module pbsd.userland.libthr.getschedparam;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.sched;

/// pthread_getschedparam from hbsd/src/lib/libthr/thread/thr_getschedparam.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status getschedparam_thread(ThreadId tid, SchedPolicy& policy,
                                                 SchedParam& param) noexcept {
    const Status sp = sched_getparam(tid, param);
    if (sp != Status::Ok) {
        return sp;
    }
    return sched_getscheduler(tid, policy);
}

} // namespace pbsd::userland::libthr
''',
"setschedparam": '''module;

export module pbsd.userland.libthr.setschedparam;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.sched;

/// pthread_setschedparam from hbsd/src/lib/libthr/thread/thr_setschedparam.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status setschedparam_thread(ThreadId tid, SchedPolicy policy,
                                                 const SchedParam& param) noexcept {
    return sched_setscheduler(tid, policy, param);
}

} // namespace pbsd::userland::libthr
''',
"getprio": '''module;

export module pbsd.userland.libthr.getprio;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_getprio from hbsd/src/lib/libthr/thread/thr_getprio.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Result<int> getprio_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::libthr
''',
"setprio": '''module;

export module pbsd.userland.libthr.setprio;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_setprio from hbsd/src/lib/libthr/thread/thr_setprio.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status setprio_thread(ThreadId tid, int prio) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)prio;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"getcpuclockid": '''module;
#include <cstdint>

export module pbsd.userland.libthr.getcpuclockid;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_getcpuclockid from hbsd/src/lib/libthr/thread/thr_getcpuclockid.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Result<std::uint64_t> getcpuclockid_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    return result_ok(0u);
}

} // namespace pbsd::userland::libthr
''',
"info": '''module;

export module pbsd.userland.libthr.info;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_get_info from hbsd/src/lib/libthr/thread/thr_info.c
export namespace pbsd::userland::libthr {

struct ThreadInfo { ThreadId id{kInvalidThread}; bool running{false}; };

[[nodiscard]] inline ThreadInfo info_for(ThreadId tid) noexcept {
    ThreadInfo info{};
    info.id = tid;
    info.running = tid != kInvalidThread;
    return info;
}

} // namespace pbsd::userland::libthr
''',
"barrierattr": '''module;

export module pbsd.userland.libthr.barrierattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_barrierattr from hbsd/src/lib/libthr/thread/thr_barrierattr.c
export namespace pbsd::userland::libthr {

struct BarrierAttr { bool pshared{false}; };

inline void barrierattr_init(BarrierAttr& a) noexcept { a = BarrierAttr{}; }

} // namespace pbsd::userland::libthr
''',
"rwlockattr": '''module;

export module pbsd.userland.libthr.rwlockattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_rwlockattr from hbsd/src/lib/libthr/thread/thr_rwlockattr.c
export namespace pbsd::userland::libthr {

struct RwLockAttr { bool pshared{false}; };

inline void rwlockattr_init(RwLockAttr& a) noexcept { a = RwLockAttr{}; }

} // namespace pbsd::userland::libthr
''',
"pshared": '''module;

export module pbsd.userland.libthr.pshared;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pshared attrs from hbsd/src/lib/libthr/thread/thr_pshared.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pshared_set(bool& flag, bool value) noexcept {
    flag = value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"multi_np": '''module;

export module pbsd.userland.libthr.multi_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_multi_np from hbsd/src/lib/libthr/thread/thr_multi_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status multi_np_enable(bool& enabled) noexcept {
    enabled = true;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"single_np": '''module;

export module pbsd.userland.libthr.single_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_single_np from hbsd/src/lib/libthr/thread/thr_single_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status single_np_enable(bool& enabled) noexcept {
    enabled = true;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"switch_np": '''module;

export module pbsd.userland.libthr.switch_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_switch_np from hbsd/src/lib/libthr/thread/thr_switch_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status switch_np_to(ThreadId from, ThreadId to) noexcept {
    if (from == kInvalidThread || to == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
''',
"suspend_np": '''module;

export module pbsd.userland.libthr.suspend_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_suspend_np from hbsd/src/lib/libthr/thread/thr_suspend_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status suspend_np_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
''',
"resume_np": '''module;

export module pbsd.userland.libthr.resume_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_resume_np from hbsd/src/lib/libthr/thread/thr_resume_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status resume_np_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
''',
"main_np": '''module;

export module pbsd.userland.libthr.main_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_main_np from hbsd/src/lib/libthr/thread/thr_main_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool main_np_is_main(ThreadId tid) noexcept {
    return tid == thread_self();
}

} // namespace pbsd::userland::libthr
''',
"sig": '''module;

export module pbsd.userland.libthr.sig;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_sigmask from hbsd/src/lib/libthr/thread/thr_sig.c
export namespace pbsd::userland::libthr {

enum class SigMaskOp : int { Block = 0, Unblock, Set };

[[nodiscard]] inline Status sigmask_op(SigMaskOp op) noexcept {
    (void)op;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"sigqueue": '''module;

export module pbsd.userland.libthr.sigqueue;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_sigqueue from hbsd/src/lib/libthr/thread/thr_sigqueue.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status sigqueue_thread(ThreadId tid, int sig) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)sig;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
''',
"autoinit": '''module;

export module pbsd.userland.libthr.autoinit;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_autoinit from hbsd/src/lib/libthr/thread/thr_autoinit.c
export namespace pbsd::userland::libthr {

inline bool g_thr_initialized{false};

inline void autoinit_once() noexcept { g_thr_initialized = true; }

} // namespace pbsd::userland::libthr
''',
"clean": '''module;

export module pbsd.userland.libthr.clean;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_clean from hbsd/src/lib/libthr/thread/thr_clean.c
export namespace pbsd::userland::libthr {

using CleanupFn = void (*)() noexcept;

[[nodiscard]] inline Status push_cleanup(CleanupFn fn) noexcept {
    (void)fn;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"event": '''module;

export module pbsd.userland.libthr.event;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_event from hbsd/src/lib/libthr/thread/thr_event.c
export namespace pbsd::userland::libthr {

struct ThreadEvent { bool pending{false}; };

inline void event_signal(ThreadEvent& e) noexcept { e.pending = true; }

} // namespace pbsd::userland::libthr
''',
"list": '''module;

export module pbsd.userland.libthr.list;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_list from hbsd/src/lib/libthr/thread/thr_list.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline bool list_contains(ThreadId tid) noexcept {
    return tid != kInvalidThread;
}

} // namespace pbsd::userland::libthr
''',
"kern": '''module;

export module pbsd.userland.libthr.kern;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_kern from hbsd/src/lib/libthr/thread/thr_kern.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status kern_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"malloc": '''module;

export module pbsd.userland.libthr.malloc;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_malloc from hbsd/src/lib/libthr/thread/thr_malloc.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status malloc_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"printf": '''module;

export module pbsd.userland.libthr.printf;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_printf from hbsd/src/lib/libthr/thread/thr_printf.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status printf_lock() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"rtld": '''module;

export module pbsd.userland.libthr.rtld;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_rtld from hbsd/src/lib/libthr/thread/thr_rtld.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status rtld_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"sleepq": '''module;

export module pbsd.userland.libthr.sleepq;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_sleepq from hbsd/src/lib/libthr/thread/thr_sleepq.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status sleepq_enqueue(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"spec": '''module;

export module pbsd.userland.libthr.spec;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_spec from hbsd/src/lib/libthr/thread/thr_spec.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status spec_set(int key, void* value) noexcept {
    (void)key;
    (void)value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
"syscalls": '''module;

export module pbsd.userland.libthr.syscalls;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_syscalls from hbsd/src/lib/libthr/thread/thr_syscalls.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status syscalls_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"umtx": '''module;

export module pbsd.userland.libthr.umtx;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_umtx from hbsd/src/lib/libthr/thread/thr_umtx.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status umtx_wait() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"ctrdtr": '''module;

export module pbsd.userland.libthr.ctrdtr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_ctrdtr from hbsd/src/lib/libthr/thread/thr_ctrdtr.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status ctrdtr_init() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
''',
"pspinlock": '''module;

export module pbsd.userland.libthr.pspinlock;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.spinlock;

/// thr_pspinlock from hbsd/src/lib/libthr/thread/thr_pspinlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pspinlock_init(SpinLock& s) noexcept {
    s.locked = false;
    s.owner = kInvalidThread;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
''',
}

usr_bin_modules = {
"host": f'''module;

export module pbsd.userland.host;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/host/host.c
export namespace pbsd::userland::usr_bin::host {{

[[nodiscard]] inline Status host_validate_name(const char* name) noexcept {{
    if (name == nullptr || name[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::usr_bin::host
''',
"last": '''module;

export module pbsd.userland.last;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/last/last.c
export namespace pbsd::userland::usr_bin::last {

[[nodiscard]] inline bool last_want_reboot(char flag) noexcept { return flag == 'x'; }

} // namespace pbsd::userland::usr_bin::last
''',
"locale": '''module;

export module pbsd.userland.locale;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locale/locale.c
export namespace pbsd::userland::usr_bin::locale {

enum class LocaleAction { List, Query, Set };

[[nodiscard]] inline LocaleAction locale_parse_flag(char c) noexcept {
    switch (c) {
    case 'a': return LocaleAction::List;
    case 'm': return LocaleAction::Query;
    default: return LocaleAction::Set;
    }
}

} // namespace pbsd::userland::usr_bin::locale
''',
"revoke": f'''module;

export module pbsd.userland.revoke;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/revoke/revoke.c
export namespace pbsd::userland::usr_bin::revoke {{

[[nodiscard]] inline Status revoke_path(const char* path) noexcept {{
    if (path == nullptr || path[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::usr_bin::revoke
''',
"limits": '''module;

export module pbsd.userland.limits;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/limits/limits.c
export namespace pbsd::userland::usr_bin::limits {

[[nodiscard]] inline bool limits_show_all(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::limits
''',
"logins": '''module;

export module pbsd.userland.logins;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/logins/logins.c
export namespace pbsd::userland::usr_bin::logins {

[[nodiscard]] inline bool logins_pretty(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::logins
''',
"fstat": '''module;

export module pbsd.userland.fstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fstat/fstat.c
export namespace pbsd::userland::usr_bin::fstat {

[[nodiscard]] inline Status fstat_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::fstat
''',
"ident": '''module;

export module pbsd.userland.ident;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ident/ident.c
export namespace pbsd::userland::usr_bin::ident {

[[nodiscard]] inline bool ident_match_keyword(const char* kw, char c) noexcept {
    return kw != nullptr && kw[0] == c;
}

} // namespace pbsd::userland::usr_bin::ident
''',
"mkdep": f'''module;

export module pbsd.userland.mkdep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkdep/mkdep.c
export namespace pbsd::userland::usr_bin::mkdep {{

[[nodiscard]] inline bool mkdep_append_suffix(const char* path) noexcept {{
    return path != nullptr && path[0] != {NUL};
}}

}} // namespace pbsd::userland::usr_bin::mkdep
''',
"rpcinfo": '''module;

export module pbsd.userland.rpcinfo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcinfo/rpcinfo.c
export namespace pbsd::userland::usr_bin::rpcinfo {

[[nodiscard]] inline bool rpcinfo_list_all(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::rpcinfo
''',
"rwall": '''module;

export module pbsd.userland.rwall;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rwall/rwall.c
export namespace pbsd::userland::usr_bin::rwall {

[[nodiscard]] inline Status rwall_message(const char* msg) noexcept {
    if (msg == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::rwall
''',
"rwho": '''module;

export module pbsd.userland.rwho;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rwho/rwho.c
export namespace pbsd::userland::usr_bin::rwho {

[[nodiscard]] inline bool rwho_all_hosts(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::rwho
''',
"rup": f'''module;

export module pbsd.userland.rup;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rup/rup.c
export namespace pbsd::userland::usr_bin::rup {{

[[nodiscard]] inline Status rup_host(const char* host) noexcept {{
    if (host == nullptr || host[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::usr_bin::rup
''',
"ruptime": '''module;

export module pbsd.userland.ruptime;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ruptime/ruptime.c
export namespace pbsd::userland::usr_bin::ruptime {

[[nodiscard]] inline bool ruptime_alive_only(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::ruptime
''',
"rusers": '''module;

export module pbsd.userland.rusers;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rusers/rusers.c
export namespace pbsd::userland::usr_bin::rusers {

[[nodiscard]] inline bool rusers_long_format(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::rusers
''',
"lastcomm": '''module;

export module pbsd.userland.lastcomm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lastcomm/lastcomm.c
export namespace pbsd::userland::usr_bin::lastcomm {

[[nodiscard]] inline bool lastcomm_user_only(char flag) noexcept { return flag == 'S'; }

} // namespace pbsd::userland::usr_bin::lastcomm
''',
"runat": f'''module;

export module pbsd.userland.runat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/runat/runat.c
export namespace pbsd::userland::usr_bin::runat {{

[[nodiscard]] inline Status runat_dir(const char* dir) noexcept {{
    if (dir == nullptr || dir[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::usr_bin::runat
''',
"quota": '''module;

export module pbsd.userland.quota;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/quota/quota.c
export namespace pbsd::userland::usr_bin::quota {

[[nodiscard]] inline bool quota_verbose(char flag) noexcept { return flag == 'v'; }

} // namespace pbsd::userland::usr_bin::quota
''',
"rctl": '''module;

export module pbsd.userland.rctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rctl/rctl.c
export namespace pbsd::userland::usr_bin::rctl {

[[nodiscard]] inline bool rctl_list_rules(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::rctl
''',
"newgrp": f'''module;

export module pbsd.userland.newgrp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/newgrp/newgrp.c
export namespace pbsd::userland::usr_bin::newgrp {{

[[nodiscard]] inline Status newgrp_name(const char* group) noexcept {{
    if (group == nullptr || group[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::usr_bin::newgrp
''',
"readelf": '''module;

export module pbsd.userland.readelf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/readelf/readelf.c
export namespace pbsd::userland::usr_bin::readelf {

[[nodiscard]] inline bool readelf_wide(char flag) noexcept { return flag == 'W'; }

} // namespace pbsd::userland::usr_bin::readelf
''',
"gencat": '''module;

export module pbsd.userland.gencat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gencat/gencat.c
export namespace pbsd::userland::usr_bin::gencat {

[[nodiscard]] inline Status gencat_output(const char* out) noexcept {
    if (out == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::gencat
''',
"mkstr": '''module;

export module pbsd.userland.mkstr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkstr/mkstr.c
export namespace pbsd::userland::usr_bin::mkstr {

[[nodiscard]] inline bool mkstr_uppercase(char flag) noexcept { return flag == 'u'; }

} // namespace pbsd::userland::usr_bin::mkstr
''',
}

bin_modules = {
"rmail": f'''module;

export module pbsd.userland.rmail;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/rmail/rmail.c
export namespace pbsd::userland::bin::rmail {{

[[nodiscard]] inline Status rmail_recipient(const char* addr) noexcept {{
    if (addr == nullptr || addr[0] == {NUL}) {{
        return Status::Invalid;
    }}
    return Status::Ok;
}}

}} // namespace pbsd::userland::bin::rmail
''',
}


def write_if_missing(path: Path, content: str) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    return True


def main() -> None:
    created: list[str] = []
    for suffix, content in libc_modules:
        path = root / "libc" / f"pbsd.userland.libc.{suffix}.cppm"
        if write_if_missing(path, content):
            created.append(str(path).replace("\\", "/"))
    for name, content in libthr_modules.items():
        path = root / "libthr" / f"pbsd.userland.libthr.{name}.cppm"
        if write_if_missing(path, content):
            created.append(str(path).replace("\\", "/"))
    for name, content in usr_bin_modules.items():
        path = root / "usr.bin" / f"pbsd.userland.{name}.cppm"
        if write_if_missing(path, content):
            created.append(str(path).replace("\\", "/"))
    for name, content in bin_modules.items():
        path = root / "bin" / f"pbsd.userland.{name}.cppm"
        if write_if_missing(path, content):
            created.append(str(path).replace("\\", "/"))
    print(f"Created {len(created)} modules")
    for p in created:
        print(p)


if __name__ == "__main__":
    main()
