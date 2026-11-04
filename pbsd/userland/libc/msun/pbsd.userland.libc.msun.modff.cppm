module;
export module pbsd.userland.libc.msun.modff;
import pbsd.userland.msun.modff;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float modff(float x, float* iptr) noexcept {
    return pbsd::userland::msun::modff_val(x, iptr);
}
}