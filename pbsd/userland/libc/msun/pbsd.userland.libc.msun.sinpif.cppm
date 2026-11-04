module;
export module pbsd.userland.libc.msun.sinpif;
import pbsd.userland.msun.sinpif;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float sinpif(float x) noexcept {
    return pbsd::userland::msun::sinpif_val(x);
}
}
