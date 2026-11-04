module;
#include <cstddef>

export module pbsd.userland.strings;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::strings {

[[nodiscard]] inline bool printable_run(const char* s, int min_len) noexcept {
    if (s == nullptr || min_len <= 0) {
        return false;
    }
    int run = 0;
    for (const char* p = s; *p; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 0x20 && c < 0x7f) {
            if (++run >= min_len) {
                return true;
            }
        } else {
            run = 0;
        }
    }
    return run >= min_len;
}

}
