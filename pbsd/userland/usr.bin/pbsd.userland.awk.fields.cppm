module;
#include <cstddef>

export module pbsd.userland.awk.fields;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::awk::fields {

[[nodiscard]] inline int count_fields(const char* line, char fs) noexcept {
    if (line == nullptr) {
        return 0;
    }
    int count = (*line == '\0') ? 0 : 1;
    for (const char* p = line; *p; ++p) {
        if (*p == fs) {
            ++count;
        }
    }
    return count;
}

}
