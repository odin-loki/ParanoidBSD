module;
#include <cstdint>

export module pbsd.geom.io;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_io.c — bio request sizing helpers.
export namespace pbsd::geom::io {

enum class BioCmd : unsigned char {
    Read = 0,
    Write = 1,
    Delete = 2,
    Flush = 3,
};

struct Bio {
    BioCmd cmd{BioCmd::Read};
    std::uint64_t offset{};
    std::uint64_t length{};
    bool completed{false};
};

[[nodiscard]] inline Status validate_bio(const Bio& b, std::uint32_t sector_size) noexcept {
    if (sector_size == 0 || (sector_size & (sector_size - 1)) != 0) {
        return Status::Invalid;
    }
    if (b.length == 0 && b.cmd != BioCmd::Flush) {
        return Status::Invalid;
    }
    if ((b.offset % sector_size) != 0 || (b.length % sector_size) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status complete(Bio& b) noexcept {
    b.completed = true;
    return Status::Ok;
}

} // namespace pbsd::geom::io
