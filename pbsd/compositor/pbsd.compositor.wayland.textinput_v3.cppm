module;

export module pbsd.compositor.wayland.textinput_v3;

import pbsd.core;

/// Burst 13 — zwp_text_input_v3 (native compositor).
export namespace pbsd::compositor::wayland::textinput_v3 {

inline constexpr const char kManagerInterface[] = "zwp_text_input_manager_v3";
inline constexpr const char kTextInputInterface[] = "zwp_text_input_v3";
inline constexpr unsigned kVersion = 3;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetTextInput = 1,
};

enum class TextInputRequest : unsigned char {
    Destroy = 0,
    Enable = 1,
    Disable = 2,
    CommitString = 3,
    SetPreeditString = 4,
    DeleteSurroundingText = 5,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetTextInput);
}

[[nodiscard]] inline bool is_text_input_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(TextInputRequest::DeleteSurroundingText);
}

} // namespace pbsd::compositor::wayland::textinput_v3
