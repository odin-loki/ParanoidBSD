module;

#include <cctype>
#include <cstdlib>
#include <cstring>

export module pbsd.userland.util.cpuset;

/// cpuset_parselist core from hbsd/src/lib/libutil/cpuset.c
export namespace pbsd::userland::util {

inline constexpr int CPUSET_PARSE_OK            = 0;
inline constexpr int CPUSET_PARSE_GETAFFINITY   = -1;
inline constexpr int CPUSET_PARSE_ERROR         = -2;
inline constexpr int CPUSET_PARSE_OUT_OF_RANGE  = -3;
inline constexpr int CPUSET_PARSE_GETDOMAIN     = -4;

struct BitsetStorage {
    unsigned char* words{nullptr};
    unsigned word_count{0};
};

namespace detail {

enum class ListState { None, Num, Dash };

[[nodiscard]] inline int parselist(const char* list, BitsetStorage* mask,
                                   int size) noexcept {
    ListState state = ListState::None;
    int lastnum = 0;
    int curnum = 0;

    for (const char* l = list; *l != '\0';) {
        if (std::isdigit(static_cast<unsigned char>(*l)) != 0) {
            curnum = std::atoi(l);
            if (curnum >= size) {
                return CPUSET_PARSE_OUT_OF_RANGE;
            }
            while (std::isdigit(static_cast<unsigned char>(*l)) != 0) {
                ++l;
            }
            switch (state) {
            case ListState::None:
                lastnum = curnum;
                state = ListState::Num;
                break;
            case ListState::Dash:
                for (; lastnum <= curnum; ++lastnum) {
                    if (lastnum < 0 || lastnum >= size) {
                        return CPUSET_PARSE_OUT_OF_RANGE;
                    }
                    const unsigned wi = static_cast<unsigned>(lastnum) / 8;
                    const unsigned bi = static_cast<unsigned>(lastnum) % 8;
                    if (wi < mask->word_count) {
                        mask->words[wi] |= static_cast<unsigned char>(1u << bi);
                    }
                }
                state = ListState::None;
                break;
            case ListState::Num:
            default:
                return CPUSET_PARSE_ERROR;
            }
            continue;
        }
        switch (*l) {
        case ',':
            switch (state) {
            case ListState::None:
                break;
            case ListState::Num: {
                if (curnum < 0 || curnum >= size) {
                    return CPUSET_PARSE_OUT_OF_RANGE;
                }
                const unsigned wi = static_cast<unsigned>(curnum) / 8;
                const unsigned bi = static_cast<unsigned>(curnum) % 8;
                if (wi < mask->word_count) {
                    mask->words[wi] |= static_cast<unsigned char>(1u << bi);
                }
                state = ListState::None;
                break;
            }
            case ListState::Dash:
                return CPUSET_PARSE_ERROR;
            }
            ++l;
            break;
        case '-':
            if (state != ListState::Num) {
                return CPUSET_PARSE_ERROR;
            }
            state = ListState::Dash;
            ++l;
            break;
        case '@':
            return CPUSET_PARSE_GETAFFINITY;
        case '/':
            return CPUSET_PARSE_GETDOMAIN;
        default:
            return CPUSET_PARSE_ERROR;
        }
    }

    if (state == ListState::Num) {
        if (curnum < 0 || curnum >= size) {
            return CPUSET_PARSE_OUT_OF_RANGE;
        }
        const unsigned wi = static_cast<unsigned>(curnum) / 8;
        const unsigned bi = static_cast<unsigned>(curnum) % 8;
        if (wi < mask->word_count) {
            mask->words[wi] |= static_cast<unsigned char>(1u << bi);
        }
    } else if (state == ListState::Dash) {
        return CPUSET_PARSE_ERROR;
    }

    return CPUSET_PARSE_OK;
}

} // namespace detail

[[nodiscard]] inline int cpuset_parselist(const char* list, BitsetStorage* mask,
                                          int size) noexcept {
    if (list == nullptr || mask == nullptr || mask->words == nullptr ||
        size <= 0) {
        return CPUSET_PARSE_ERROR;
    }
    std::memset(mask->words, 0, mask->word_count);
    return detail::parselist(list, mask, size);
}

} // namespace pbsd::userland::util
