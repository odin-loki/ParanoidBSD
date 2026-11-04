module;

export module pbsd.userland.msun.truncf;

import pbsd.userland.msun.ceilf;
import pbsd.userland.msun.floorf;

/// truncf from hbsd/src/lib/msun/src/s_truncf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float truncf(float x) noexcept {
    return x >= 0.0f ? floorf(x) : ceilf(x);
}

} // namespace pbsd::userland::msun
