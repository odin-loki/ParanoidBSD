module;
export module pbsd.userland.libc.msun.fmodf;
import pbsd.userland.msun.fmodf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float fmodf(float x, float y) noexcept {
    return pbsd::userland::msun::fmodf_val(x, y);
}
}