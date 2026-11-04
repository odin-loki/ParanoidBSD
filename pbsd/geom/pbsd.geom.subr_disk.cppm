module;
#include <cstdint>

export module pbsd.geom.subr_disk;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_disk.c helpers — disk attribute scaffold.
export namespace pbsd::geom::subr_disk {

struct Attr {
    std::uint32_t sectorsize{512};
    std::uint64_t mediasize{};
    unsigned fwsectors{63};
    unsigned fwheads{255};
};

[[nodiscard]] inline Status validate_attr(const Attr& a) noexcept {
    if (a.sectorsize == 0 || (a.sectorsize & (a.sectorsize - 1)) != 0) {
        return Status::Invalid;
    }
    if (a.mediasize == 0 || (a.mediasize % a.sectorsize) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t sector_count(const Attr& a) noexcept {
    if (validate_attr(a) != Status::Ok) {
        return 0;
    }
    return a.mediasize / a.sectorsize;
}

} // namespace pbsd::geom::subr_disk
