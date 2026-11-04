module;
#include <cstdint>

export module pbsd.kernel.sglist;

export import pbsd.core;

/// Freestanding port of `sys/sglist.h` / `kern/subr_sglist.c`.
export namespace pbsd::kernel::sglist {

struct Seg {
    std::uint64_t paddr{};
    std::size_t   len{};
};

struct List {
    Seg*      segs{nullptr};
    unsigned  refs{1};
    unsigned short nseg{};
    unsigned short maxseg{};
};

inline void init(List& sg, unsigned short maxsegs, Seg* segs) noexcept {
    sg.segs = segs;
    sg.nseg = 0;
    sg.maxseg = maxsegs;
    sg.refs = 1;
}

inline void reset(List& sg) noexcept { sg.nseg = 0; }

[[nodiscard]] inline Status append(List& sg, std::uint64_t paddr, std::size_t len) noexcept {
    if (sg.segs == nullptr || sg.nseg >= sg.maxseg || len == 0) {
        return Status::Invalid;
    }
    sg.segs[sg.nseg].paddr = paddr;
    sg.segs[sg.nseg].len = len;
    ++sg.nseg;
    return Status::Ok;
}

} // namespace pbsd::kernel::sglist
