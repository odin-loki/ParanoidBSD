module;

export module pbsd.userland.libc.stdlib.getopt;

/// getopt state from hbsd/src/lib/libc/stdlib/getopt.c
export namespace pbsd::userland::libc {

struct GetoptState {
    int opterr{1};
    int optind{1};
    int optopt{0};
    int optreset{0};
    char* optarg{nullptr};
};

[[nodiscard]] inline int getopt(int argc, char* const argv[], const char* optstring,
                                GetoptState& st) noexcept {
    if (st.optreset != 0 || st.optind >= argc) {
        return -1;
    }
    const char* place = argv[st.optind];
    if (place == nullptr || place[0] != '-' || place[1] == '\0') {
        return -1;
    }
    if (place[1] == '-' && place[2] == '\0') {
        ++st.optind;
        return -1;
    }
    const char opt = place[1];
    for (const char* p = optstring; *p != '\0'; ++p) {
        if (*p == ':') {
            continue;
        }
        if (*p == opt) {
            ++st.optind;
            st.optopt = static_cast<unsigned char>(opt);
            return st.optopt;
        }
    }
    st.optopt = static_cast<unsigned char>(opt);
    return '?';
}

} // namespace pbsd::userland::libc
