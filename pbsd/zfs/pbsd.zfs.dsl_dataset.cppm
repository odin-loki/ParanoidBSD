module;
#include <cstdint>

export module pbsd.zfs.dsl_dataset;

import pbsd.core;

/// PROVENANCE: openzfs dsl_dataset — dataset handle scaffold.
export namespace pbsd::zfs::dsl_dataset {

struct Dataset {
    std::uint64_t ds_object{};
    std::uint64_t create_txg{};
    bool snapshot{false};
    bool open{false};
};

[[nodiscard]] inline Status open_ds(Dataset& d, std::uint64_t obj, std::uint64_t txg) noexcept {
    if (obj == 0 || txg == 0) {
        return Status::Invalid;
    }
    d.ds_object = obj;
    d.create_txg = txg;
    d.open = true;
    return Status::Ok;
}

[[nodiscard]] inline Status mark_snapshot(Dataset& d) noexcept {
    if (!d.open) {
        return Status::Invalid;
    }
    d.snapshot = true;
    return Status::Ok;
}

[[nodiscard]] inline Status close_ds(Dataset& d) noexcept {
    d = Dataset{};
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl_dataset
