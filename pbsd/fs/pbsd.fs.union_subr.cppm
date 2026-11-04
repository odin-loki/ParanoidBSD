module;
#include <cstdint>

export module pbsd.fs.union_subr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs/union_subr.c — union layer helpers.
export namespace pbsd::fs::union_subr {

inline constexpr unsigned kMaxLayers = 32;

struct Layer {
    unsigned id{};
    bool upper{false};
    bool whiteout{false};
};

[[nodiscard]] inline Status push_layer(Layer* stack, unsigned& count, Layer layer) noexcept {
    if (count >= kMaxLayers) {
        return Status::Busy;
    }
    stack[count++] = layer;
    return Status::Ok;
}

[[nodiscard]] inline Status resolve_whiteout(const Layer* stack, unsigned count) noexcept {
    for (unsigned i = 0; i < count; ++i) {
        if (stack[i].whiteout) {
            return Status::NotFound;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::fs::union_subr
