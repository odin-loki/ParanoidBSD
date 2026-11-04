module;

#include <cstddef>

export module pbsd.userland.unifdef;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/unifdef/unifdef.c — directive prefix scan.
export namespace pbsd::userland::usr_bin::unifdef {

enum class Directive { If, Ifdef, Ifndef, Elif, Else, Endif, Unknown };

[[nodiscard]] inline Directive classify_directive(const char* line) noexcept {
    if (line == nullptr) {
        return Directive::Unknown;
    }
    if (line[0] != '#' || line[1] != ' ') {
        return Directive::Unknown;
    }
    const char* p = line + 2;
    struct Pair {
        const char* word;
        Directive dir;
    };
    static constexpr Pair kPairs[] = {
        {"if ", Directive::If},       {"ifdef ", Directive::Ifdef},
        {"ifndef ", Directive::Ifndef}, {"elif ", Directive::Elif},
        {"else", Directive::Else},    {"endif", Directive::Endif},
    };
    for (const auto& pair : kPairs) {
        std::size_t i = 0;
        while (pair.word[i] != '\0' && p[i] == pair.word[i]) {
            ++i;
        }
        if (pair.word[i] == '\0') {
            return pair.dir;
        }
    }
    return Directive::Unknown;
}

} // namespace pbsd::userland::usr_bin::unifdef
