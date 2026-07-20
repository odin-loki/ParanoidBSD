module;

#include <cstddef>

export module pbsd.userland.rtld.libmap;

/// libmap.conf concepts from hbsd/src/libexec/rtld-elf/libmap.c
export namespace pbsd::userland::rtld {

struct LibmapEntry {
    const char* from{nullptr};
    const char* to{nullptr};
};

namespace detail {
[[nodiscard]] inline bool cstr_eq(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return a == b;
    }
    while (*a != '\0' && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}
} // namespace detail

[[nodiscard]] inline const char* libmap_lookup(const LibmapEntry* table, std::size_t count,
                                                 const char* name) noexcept {
    if (table == nullptr || name == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (detail::cstr_eq(table[i].from, name)) {
            return table[i].to;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::rtld
