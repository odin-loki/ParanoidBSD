module;
#include <cstddef>

export module pbsd.userland.libc.gen.stringlist;

export import pbsd.core;

/// stringlist from hbsd/src/lib/libc/gen/stringlist.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kChunkSize = 20;

struct StringList {
    unsigned cur{0};
    unsigned max{kChunkSize};
};

[[nodiscard]] inline Status sl_init(StringList& sl) noexcept {
    sl.cur = 0;
    sl.max = kChunkSize;
    return Status::Ok;
}

[[nodiscard]] inline Status sl_need(StringList& sl, unsigned extra) noexcept {
    if (sl.cur + extra > sl.max) {
        sl.max += kChunkSize;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned sl_count(StringList const& sl) noexcept {
    return sl.cur;
}

} // namespace pbsd::userland::libc
