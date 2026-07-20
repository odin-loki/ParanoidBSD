module;

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>

export module pbsd.userland.jail.param;

import pbsd.userland.jail;
import pbsd.userland.libc.stdlib;
import pbsd.core;

/// jailparam_* helpers from hbsd/src/lib/libjail/jail.c + jail.h
export namespace pbsd::userland::jail {

inline constexpr unsigned kErrMsgLen = kJailErrMsgLen;

inline char jail_errmsg[kErrMsgLen]{};

inline constexpr const char* kBoolFalse = "false";
inline constexpr const char* kBoolTrue  = "true";

inline constexpr const char* kJailsysValues[] = {"disable", "new", "inherit"};
inline constexpr unsigned kJailsysValueCount = 3;

namespace detail {

[[nodiscard]] inline int strncasecmp_local(const char* a, const char* b,
                                           unsigned n) noexcept {
    for (unsigned i = 0; i < n; ++i) {
        const unsigned ca = static_cast<unsigned char>(a[i]);
        const unsigned cb = static_cast<unsigned char>(b[i]);
        if (ca == '\0' || cb == '\0') {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        const int da = std::tolower(ca);
        const int db = std::tolower(cb);
        if (da != db) {
            return da - db;
        }
    }
    return 0;
}

[[nodiscard]] inline bool import_enum(const char* const* values, unsigned nvalues,
                                      const char* valstr, unsigned valsize,
                                      int& out) noexcept {
    for (unsigned i = 0; i < nvalues; ++i) {
        if (valsize == std::strlen(values[i]) &&
            strncasecmp_local(valstr, values[i], valsize) == 0) {
            out = static_cast<int>(i);
            return true;
        }
    }
    char* ep = nullptr;
    const long v = pbsd::userland::libc::strtol(valstr, &ep, 10);
    if (ep == valstr + valsize) {
        out = static_cast<int>(v);
        return true;
    }
    return false;
}

} // namespace detail

[[nodiscard]] inline Status param_init(JailParam& jp, const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    jp.name = name;
    jp.value = nullptr;
    jp.value_len = 0;
    jp.elem_len = 0;
    jp.ctl_type = 0;
    jp.struct_type = 0;
    jp.flags = ParamFlag::KeyValue;
    return Status::Ok;
}

[[nodiscard]] inline Status param_import(JailParam& jp,
                                         const char* value) noexcept {
    if (!jp.name) {
        return Status::Invalid;
    }
    if (value == nullptr) {
        return Status::Ok;
    }
    jp.value = value;
    jp.value_len = static_cast<unsigned>(std::strlen(value) + 1);
    return Status::Ok;
}

[[nodiscard]] inline const char* export_bool(bool v) noexcept {
    return v ? kBoolTrue : kBoolFalse;
}

[[nodiscard]] inline Status parse_bool(const char* value, bool& out) noexcept {
    if (value == nullptr) {
        return Status::Invalid;
    }
    if (std::strcmp(value, kBoolFalse) == 0) {
        out = false;
        return Status::Ok;
    }
    if (std::strcmp(value, kBoolTrue) == 0) {
        out = true;
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline Status validate_param_list(JailParam* params,
                                              unsigned count) noexcept {
    if (params == nullptr || count == 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < count; ++i) {
        if (params[i].name == nullptr || params[i].name[0] == '\0') {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status parse_jailsys(const char* value, int& out) noexcept {
    if (value == nullptr) {
        return Status::Invalid;
    }
    const unsigned len = static_cast<unsigned>(std::strlen(value));
    if (!detail::import_enum(kJailsysValues, kJailsysValueCount, value, len, out)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status param_import_raw(JailParam& jp, const void* value,
                                             unsigned valuelen) noexcept {
    if (value == nullptr && valuelen != 0) {
        return Status::Invalid;
    }
    jp.value = value;
    jp.value_len = valuelen;
    jp.flags = static_cast<ParamFlag>(static_cast<unsigned>(jp.flags) |
                                      static_cast<unsigned>(ParamFlag::RawValue));
    return Status::Ok;
}

[[nodiscard]] inline Status param_free(JailParam& jp) noexcept {
    if ((static_cast<unsigned>(jp.flags) &
         static_cast<unsigned>(ParamFlag::RawValue)) == 0) {
        jp.value = nullptr;
    }
    jp.name = nullptr;
    jp.value_len = 0;
    jp.elem_len = 0;
    return Status::Ok;
}

/// noname() — derive "no" boolean counterpart parameter name.
[[nodiscard]] inline Status make_noname(const char* name, char* out,
                                        unsigned outlen) noexcept {
    if (name == nullptr || out == nullptr || outlen == 0) {
        return Status::Invalid;
    }
    const char* dot = std::strrchr(name, '.');
    if (dot != nullptr) {
        const int prefix = static_cast<int>(dot - name);
        const int need = prefix + 4 + static_cast<int>(std::strlen(dot + 1)) + 1;
        if (static_cast<unsigned>(need) > outlen) {
            return Status::Invalid;
        }
        ::snprintf(out, outlen, "%.*s.no%s", prefix, name, dot + 1);
    } else {
        const int need = 2 + static_cast<int>(std::strlen(name)) + 1;
        if (static_cast<unsigned>(need) > outlen) {
            return Status::Invalid;
        }
        ::snprintf(out, outlen, "no%s", name);
    }
    return Status::Ok;
}

/// nononame() — strip "no" prefix from boolean parameter name.
[[nodiscard]] inline Status make_nononame(const char* name, char* out,
                                          unsigned outlen) noexcept {
    if (name == nullptr || out == nullptr || outlen == 0) {
        return Status::Invalid;
    }
    const char* base = std::strrchr(name, '.');
    base = base != nullptr ? base + 1 : name;
    if (base[0] != 'n' || base[1] != 'o') {
        return Status::Invalid;
    }
    const char* dot = std::strrchr(name, '.');
    if (dot != nullptr) {
        const int prefix = static_cast<int>(dot - name);
        const int need = prefix + 1 + static_cast<int>(std::strlen(base + 2)) + 1;
        if (static_cast<unsigned>(need) > outlen) {
            return Status::Invalid;
        }
        ::snprintf(out, outlen, "%.*s.%s", prefix, name, base + 2);
    } else {
        const int need = static_cast<int>(std::strlen(base + 2)) + 1;
        if (static_cast<unsigned>(need) > outlen) {
            return Status::Invalid;
        }
        ::snprintf(out, outlen, "%s", base + 2);
    }
    return Status::Ok;
}

/// kvname() — extract key portion before first '.' in parameter name.
[[nodiscard]] inline Status make_kvname(const char* name, char* out,
                                        unsigned outlen) noexcept {
    if (name == nullptr || out == nullptr || outlen == 0) {
        return Status::Invalid;
    }
    const char* dot = std::strchr(name, '.');
    if (dot == nullptr) {
        return Status::Invalid;
    }
    const unsigned len = static_cast<unsigned>(dot - name);
    if (len + 1 > outlen) {
        return Status::Invalid;
    }
    std::memcpy(out, name, len);
    out[len] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::jail
