module;
#include <cstdint>

export module pbsd.kernel.subr_linker;

import pbsd.core;
import pbsd.kernel.linker;

/// PROVENANCE: hbsd/src/sys/kern/kern_linker.c — kld load/unload subr.
export namespace pbsd::kernel::subr_linker {

[[nodiscard]] inline Status load_module(linker::Table& t, const char* name,
                                        std::uint32_t id) noexcept {
    return linker::load(t, name, id);
}

[[nodiscard]] inline Status unload_module(linker::Table& t,
                                          std::uint32_t id) noexcept {
    return linker::unload(t, id);
}

[[nodiscard]] inline unsigned loaded_count(linker::Table const& t) noexcept {
    unsigned n = 0;
    for (unsigned i = 0; i < t.count; ++i) {
        if (t.mods[i].loaded) {
            ++n;
        }
    }
    return n;
}

} // namespace pbsd::kernel::subr_linker
