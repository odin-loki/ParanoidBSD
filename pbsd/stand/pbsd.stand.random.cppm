module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.random;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/random.c
export namespace pbsd::stand::random {

[[nodiscard]] inline unsigned mix(unsigned a, unsigned b) noexcept {
    return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
}

} // namespace pbsd::stand::random
