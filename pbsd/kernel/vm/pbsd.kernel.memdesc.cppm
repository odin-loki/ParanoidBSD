module;
#include <cstdint>

export module pbsd.kernel.memdesc;

export import pbsd.core;

/// Freestanding port of `sys/memdesc.h` — physical memory descriptor helpers.
export namespace pbsd::kernel::memdesc {

inline constexpr unsigned kMemdescMaxSegs = 64u;

enum class Type : unsigned {
    Unknown = 0,
    Ram     = 1,
    Reserved = 2,
    Acpi    = 3,
};

struct Segment {
    std::uint64_t base{};
    std::uint64_t size{};
    Type          type{Type::Unknown};
};

struct Desc {
    Segment segs[kMemdescMaxSegs]{};
    unsigned count{};
};

[[nodiscard]] inline Status validate_segment(std::uint64_t base,
                                             std::uint64_t size) noexcept {
    if (size == 0) {
        return Status::Invalid;
    }
    if (base + size < base) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(Desc& md) noexcept {
    md.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status append(Desc& md, Segment seg) noexcept {
    if (md.count >= kMemdescMaxSegs) {
        return Status::Busy;
    }
    if (validate_segment(seg.base, seg.size) != Status::Ok) {
        return Status::Invalid;
    }
    md.segs[md.count++] = seg;
    return Status::Ok;
}

} // namespace pbsd::kernel::memdesc
