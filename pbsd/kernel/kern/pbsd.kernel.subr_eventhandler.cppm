module;
#include <cstdint>

export module pbsd.kernel.subr_eventhandler;

import pbsd.core;
import pbsd.kernel.eventhandler;

/// Freestanding port of `kern/subr_eventhandler.c` helpers.
export namespace pbsd::kernel::subr_eventhandler {

[[nodiscard]] inline Status attach(eventhandler::ListStub& list,
                                    eventhandler::HandlerFn fn,
                                    void* arg,
                                    int pri) noexcept {
    if (list.name == nullptr) {
        return Status::Invalid;
    }
    return eventhandler::register_handler(list, fn, arg, pri);
}

} // namespace pbsd::kernel::subr_eventhandler
