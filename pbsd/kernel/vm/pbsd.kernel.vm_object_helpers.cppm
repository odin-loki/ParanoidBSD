module;
#include <cstdint>

export module pbsd.kernel.vm_object_helpers;

import pbsd.core;
import pbsd.kernel.vm_object;

/// Freestanding port of `vm/vm_object.c`.
export namespace pbsd::kernel::vm_object_helpers {

using namespace pbsd::kernel::vm_object;

[[nodiscard]] inline Status extend_size(VmObjectHeader& obj, std::uint64_t delta) noexcept {
    if (is_dead(obj.flags)) {
        return Status::Invalid;
    }
    obj.size += delta;
    return Status::Ok;
}

[[nodiscard]] inline bool covers_offset(const VmObjectHeader& obj, std::uint64_t off) noexcept {
    return off < obj.size;
}

} // namespace pbsd::kernel::vm_object_helpers
