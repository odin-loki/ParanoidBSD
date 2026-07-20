module;

#include <cstddef>

export module pbsd.userland.morse;

export import pbsd.core;

/// Morse table from hbsd/src/usr.bin/morse/morse.c
export namespace pbsd::userland::usr_bin::morse {

struct MorseEntry {
    char inch;
    const char* code;
};

inline constexpr MorseEntry kTable[] = {
    {'a', ".-"},   {'b', "-..."}, {'c', "-.-."}, {'d', "-.."},  {'e', "."},
    {'f', "..-."}, {'g', "--."},  {'h', "...."}, {'i', ".."},   {'j', ".---"},
    {'k', "-.-"},  {'l', ".-.."}, {'m', "--"},   {'n', "-."},   {'o', "---"},
    {'p', ".--."}, {'q', "--.-"}, {'r', ".-."},  {'s', "..."},  {'t', "-"},
    {'u', "..-"},  {'v', "...-"}, {'w', ".--"},  {'x', "-..-"}, {'y', "-.--"},
    {'z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
    {',', "--..--"}, {'.', ".-.-.-"}, {'?', "..--.."}, {'\0', ""},
};

[[nodiscard]] inline const char* lookup(char ch) noexcept {
    char lower = ch;
    if (lower >= 'A' && lower <= 'Z') {
        lower = static_cast<char>(lower - 'A' + 'a');
    }
    for (const auto& e : kTable) {
        if (e.inch == lower) {
            return e.code;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::size_t encode_char(char ch, char* out, std::size_t cap) noexcept {
    const char* code = lookup(ch);
    if (code == nullptr || out == nullptr || cap == 0) {
        return 0;
    }
    std::size_t n = 0;
    while (code[n] != '\0' && n + 1 < cap) {
        out[n] = code[n];
        ++n;
    }
    if (n < cap) {
        out[n] = '\0';
    }
    return n;
}

} // namespace pbsd::userland::usr_bin::morse
