module;

export module pbsd.stand.chain;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/chain.c
export namespace pbsd::stand::chain {

enum class Stage : unsigned char {
    Firmware = 0,
    Loader = 1,
    Kernel = 2,
};

struct Link {
    Stage stage{Stage::Firmware};
    const char* component{};
};

[[nodiscard]] inline Status advance(Link& link, Stage next) noexcept {
    if (static_cast<unsigned>(next) <= static_cast<unsigned>(link.stage)) {
        return Status::Invalid;
    }
    link.stage = next;
    return Status::Ok;
}

} // namespace pbsd::stand::chain
