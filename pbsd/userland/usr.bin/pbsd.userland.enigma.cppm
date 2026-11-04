module;

#include <cstdint>

export module pbsd.userland.enigma;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/enigma/enigma.c — rotor shuffle constants.
export namespace pbsd::userland::usr_bin::enigma {

inline constexpr int kRotorSize = 256;
inline constexpr int kMask = 0377;
inline constexpr int kSeedBase = 123;
inline constexpr int kPrime = 65521;

[[nodiscard]] inline std::uint32_t mix_seed(std::uint32_t seed, unsigned char ch,
                                            int idx) noexcept {
    return seed * 5U + static_cast<std::uint32_t>(ch) + static_cast<std::uint32_t>(idx);
}

[[nodiscard]] inline int shuffle_index(std::uint32_t rnd, int k) noexcept {
    return static_cast<int>((rnd & kMask) % static_cast<unsigned>(k + 1));
}

[[nodiscard]] inline void identity_rotor(unsigned char deck[kRotorSize]) noexcept {
    for (int i = 0; i < kRotorSize; ++i) {
        deck[i] = static_cast<unsigned char>(i);
    }
}

} // namespace pbsd::userland::usr_bin::enigma
