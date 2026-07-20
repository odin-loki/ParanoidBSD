module;
export module pbsd.userland.libc.msun.sqrtf;
import pbsd.userland.msun.sqrtf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float sqrtf(float x) noexcept {
    return pbsd::userland::msun::sqrtf_val(x);
}
}