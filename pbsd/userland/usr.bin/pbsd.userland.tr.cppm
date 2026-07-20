module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.tr;

export import pbsd.core;

/// Port helpers from hbsd/src/usr.bin/tr/tr.c — complement/delete/squeeze + map table.
export namespace pbsd::userland::usr_bin::tr {

struct Options {
    bool complement{false}; // -c/-C
    bool delete_set{false}; // -d
    bool squeeze{false};    // -s
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'c':
            case 'C':
                opt.complement = true;
                break;
            case 'd':
                opt.delete_set = true;
                break;
            case 's':
                opt.squeeze = true;
                break;
            case 'u':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Build identity map[256]; then apply string1→string2 pairwise (ASCII scaffold).
inline void build_map(std::uint8_t map[256], const char* s1, const char* s2) noexcept {
    for (int i = 0; i < 256; ++i) {
        map[i] = static_cast<std::uint8_t>(i);
    }
    if (s1 == nullptr || s2 == nullptr) {
        return;
    }
    std::size_t i = 0;
    for (; s1[i] != '\0' && s2[i] != '\0'; ++i) {
        map[static_cast<std::uint8_t>(s1[i])] = static_cast<std::uint8_t>(s2[i]);
    }
    // If s2 shorter, last char of s2 fills remainder of s1 (BSD tr)
    if (s1[i] != '\0' && s2[0] != '\0') {
        std::uint8_t last = 0;
        for (const char* p = s2; *p; ++p) {
            last = static_cast<std::uint8_t>(*p);
        }
        for (; s1[i] != '\0'; ++i) {
            map[static_cast<std::uint8_t>(s1[i])] = last;
        }
    }
}

[[nodiscard]] inline std::size_t translate(const std::uint8_t map[256], const char* in,
                                           std::size_t in_len, char* out,
                                           std::size_t out_cap) noexcept {
    if (in == nullptr || out == nullptr || out_cap == 0) {
        return 0;
    }
    std::size_t o = 0;
    for (std::size_t i = 0; i < in_len && o + 1 < out_cap; ++i) {
        out[o++] = static_cast<char>(map[static_cast<std::uint8_t>(in[i])]);
    }
    out[o] = '\0';
    return o;
}

} // namespace pbsd::userland::usr_bin::tr
