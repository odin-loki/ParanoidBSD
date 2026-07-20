module;
export module pbsd.userland.libc.msun.cbrtf;
import pbsd.userland.msun.cbrtf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float cbrtf(float x) noexcept {
    return pbsd::userland::msun::cbrtf_val(x);
}
}