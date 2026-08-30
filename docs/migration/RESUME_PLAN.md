# PBSD resume notes

Single path: **DeepSeek** via `pbsd.py` (Flash → Pro, max effort). No Cursor. No mechanical IR sweep.

## Before a burn

1. Top up DeepSeek balance (API returns 402 if empty).
2. Ensure `secrets/api-keys` has `DEEPSEEK_API_KEY` (gitignored).
3. Smoke:

```powershell
python pbsd.py --self-test
python pbsd.py --dry-run --scope bin --limit 10
python pbsd.py --scope bin --limit 5
```

4. Full / WSL: `.\scripts\run_agent_port_deepseek.ps1` or `bash scripts/wsl/restart_pbsd.sh`.

## Work queue

Prefer `refusals.jsonl` when present; otherwise stubbed / `NEEDS-REVIEW` rows in `docs/migration/batch_progress.json`.

## Outputs

- Progress: `docs/migration/batch_progress.json`
- Failures (resume later): `docs/migration/clang_port/agent_port_failures.jsonl`
- Cost: `docs/migration/clang_port/agent_port_cost.jsonl`
