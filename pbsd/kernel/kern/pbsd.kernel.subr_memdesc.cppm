module;
#include <cstdint>

export module pbsd.kernel.subr_memdesc;

import pbsd.core;
import pbsd.kernel.memdesc;

/// Freestanding port of `kern/subr_memdesc.c` — memdesc merge/split helpers.
export namespace pbsd::kernel::subr_memdesc {

[[nodiscard]] inline bool segments_adjacent(const memdesc::Segment& a,
                                            const memdesc::Segment& b) noexcept {
    return a.base + a.size == b.base;
}

[[nodiscard]] inline Status try_merge(memdesc::Segment& a,
                                      const memdesc::Segment& b) noexcept {
    if (a.type != b.type) {
        return Status::Protocol;
    }
    if (!segments_adjacent(a, b)) {
        return Status::NotFound;
    }
    a.size += b.size;
    return Status::Ok;
}

[[nodiscard]] inline Status find_ram(const memdesc::Desc& md,
                                     std::uint64_t addr) noexcept {
    for (unsigned i = 0; i < md.count; ++i) {
        const auto& s = md.segs[i];
        if (s.type != memdesc::Type::Ram) {
            continue;
        }
        if (addr >= s.base && addr < s.base + s.size) {
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

} // namespace pbsd::kernel::subr_memdesc
