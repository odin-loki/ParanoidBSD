module;

#include <cstdint>

export module pbsd.userland.rtld.linkmap;

/// Link map entry concept from hbsd/src/libexec/rtld-elf/map_object.c
export namespace pbsd::userland::rtld {

struct LinkMap {
    std::uintptr_t l_addr{0};
    const char* l_name{nullptr};
    std::uintptr_t l_ld{0};
    LinkMap* l_next{nullptr};
    LinkMap* l_prev{nullptr};
};

struct ObjEntry {
    LinkMap linkmap{};
    unsigned ref_count{0};
    bool main_program{false};
};

[[nodiscard]] inline bool is_main_object(const ObjEntry& obj) noexcept {
    return obj.main_program;
}

} // namespace pbsd::userland::rtld
