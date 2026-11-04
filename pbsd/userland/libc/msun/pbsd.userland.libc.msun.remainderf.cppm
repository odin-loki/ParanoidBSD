module;
export module pbsd.userland.libc.msun.remainderf;
import pbsd.userland.msun.remainderf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float remainderf(float x, float y) noexcept {
    return pbsd::userland::msun::remainderf_val(x, y);
}
}