module;
export module pbsd.userland.libc.msun.sinhf;
import pbsd.userland.msun.sinhf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float sinhf(float x) noexcept {
    return pbsd::userland::msun::sinhf_val(x);
}
}