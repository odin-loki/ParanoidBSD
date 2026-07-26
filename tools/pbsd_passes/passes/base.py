"""Pass ABC."""
from __future__ import annotations

from abc import ABC, abstractmethod

from ..schema import PassResult
from ..unit import TranslationUnit


class Pass(ABC):
    name: str = "base"
    tier: int = 0

    def precondition(self, unit: TranslationUnit) -> bool:
        return True

    @abstractmethod
    def apply(self, unit: TranslationUnit) -> PassResult:
        raise NotImplementedError
