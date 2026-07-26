"""Translation unit buffer with comment/string-aware scanning helpers."""
from __future__ import annotations

import re
from dataclasses import dataclass, field


@dataclass
class TranslationUnit:
    path: str
    text: str
    original_text: str = ""
    meta: dict = field(default_factory=dict)

    def __post_init__(self) -> None:
        if not self.original_text:
            self.original_text = self.text

    def line_col(self, index: int) -> tuple[int, int]:
        if index < 0:
            index = 0
        if index > len(self.text):
            index = len(self.text)
        line = self.text.count("\n", 0, index) + 1
        last_nl = self.text.rfind("\n", 0, index)
        col = index - last_nl
        return line, col

    def snippet_at(self, index: int, radius: int = 60) -> str:
        start = max(0, index - radius)
        end = min(len(self.text), index + radius)
        s = self.text[start:end].replace("\n", "\\n")
        return s[:120]

    def enclosing_function(self, index: int) -> str:
        # Best-effort: last function-like declarator before index.
        head = self.text[:index]
        matches = list(
            re.finditer(
                r"(?:^|\n)([A-Za-z_][\w\s\*]*)\b([A-Za-z_]\w*)\s*\([^;]*\)\s*\{",
                head,
            )
        )
        if not matches:
            return "<file>"
        return matches[-1].group(2)

    def mask_strings_comments(self) -> str:
        """Return text with strings/comments replaced by spaces (same length)."""
        src = self.text
        out = list(src)
        i = 0
        n = len(src)
        while i < n:
            c = src[i]
            if c == "/" and i + 1 < n and src[i + 1] == "/":
                while i < n and src[i] != "\n":
                    out[i] = " "
                    i += 1
                continue
            if c == "/" and i + 1 < n and src[i + 1] == "*":
                out[i] = out[i + 1] = " "
                i += 2
                while i + 1 < n and not (src[i] == "*" and src[i + 1] == "/"):
                    out[i] = " " if src[i] != "\n" else "\n"
                    i += 1
                if i + 1 < n:
                    out[i] = out[i + 1] = " "
                    i += 2
                continue
            if c == '"':
                out[i] = " "
                i += 1
                while i < n:
                    if src[i] == "\\":
                        out[i] = " "
                        if i + 1 < n:
                            out[i + 1] = " "
                            i += 2
                            continue
                    if src[i] == '"':
                        out[i] = " "
                        i += 1
                        break
                    out[i] = " " if src[i] != "\n" else "\n"
                    i += 1
                continue
            if c == "'":
                out[i] = " "
                i += 1
                while i < n:
                    if src[i] == "\\":
                        out[i] = " "
                        if i + 1 < n:
                            out[i + 1] = " "
                            i += 2
                            continue
                    if src[i] == "'":
                        out[i] = " "
                        i += 1
                        break
                    out[i] = " "
                    i += 1
                continue
            i += 1
        return "".join(out)
