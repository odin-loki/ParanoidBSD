module;
#include <cstddef>

export module pbsd.userland.install;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/stand/common/install.c — path helpers (logic-only).
export namespace pbsd::userland::sbin::install {

[[nodiscard]] inline StatusOnly setpath(char** dest, const char* val,
                                        char* buf, std::size_t cap) noexcept {
    if (dest == nullptr || val == nullptr || buf == nullptr || cap == 0) {
        return status_err(Status::Invalid);
    }
    std::size_t len = hosted::cstrlen(val);
    const bool rel = val[0] != '/';
    const std::size_t need = len + (rel ? 1u : 0u) + 1u;
    if (need > cap) {
        return status_err(Status::Invalid);
    }
    std::size_t pos = 0;
    if (rel) {
        buf[pos++] = '/';
    }
    for (std::size_t i = 0; i < len; ++i) {
        buf[pos++] = val[i];
    }
    buf[pos] = '\0';
    *dest = buf;
    return status_ok();
}

[[nodiscard]] inline int count_csv_paths(const char* val) noexcept {
    if (val == nullptr || val[0] == '\0') {
        return 0;
    }
    int count = 1;
    for (const char* p = val; *p != '\0'; ++p) {
        if (*p == ',') {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] inline const char* next_csv_elem(const char*& val) noexcept {
    if (val == nullptr) {
        return nullptr;
    }
    const char* elem = val;
    while (*val != '\0' && *val != ',') {
        ++val;
    }
    if (*val == ',') {
        ++val;
    }
    return elem;
}

} // namespace pbsd::userland::sbin::install
