module;
#include <cstdint>

export module pbsd.geom.orphan;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_orphan.c — GEOM orphan consumer helpers.
export namespace pbsd::geom::orphan {

struct ConsumerState {
    unsigned acw{};
    unsigned acr{};
    bool dying{false};
};

[[nodiscard]] inline Status mark_dying(ConsumerState& cs) noexcept {
    if (cs.dying) {
        return Status::Busy;
    }
    cs.dying = true;
    return Status::Ok;
}

[[nodiscard]] inline Status can_destroy(const ConsumerState& cs) noexcept {
    if (!cs.dying) {
        return Status::Invalid;
    }
    if (cs.acw != 0 || cs.acr != 0) {
        return Status::Busy;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::orphan
