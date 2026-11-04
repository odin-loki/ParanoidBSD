# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tier 4 — verification wrappers + clang-tidy integration markers."""
from __future__ import annotations

from ..schema import PassResult
from ..unit import TranslationUnit
from .base import Pass


class IrOracleMarkerPass(Pass):
    name = "ir_oracle_marker"
    tier = 4

    def apply(self, unit: TranslationUnit) -> PassResult:
        unit.meta["ir_oracle_requested"] = True
        return PassResult.unchanged(unit.text)


TIER4_PASSES: list[Pass] = [IrOracleMarkerPass()]
