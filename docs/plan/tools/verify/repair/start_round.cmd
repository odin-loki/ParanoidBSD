@echo off
rem start_round.cmd <round> <repair-budget-usd> <review-budget-usd>   One repair round (round_run.sh): state -> todo -> GLM repair -> re-verify -> DeepSeek review -> state. Resumable.
rem Data: C:\ParanoidBSD\mined-data\repair<N> (repair), verify-repair<N> (re-verify + xcheck). Stop cleanly: create repair<N>\STOP or verify-repair<N>\xcheck\STOP.
set N=%1
set RB=%2
set VB=%3
set R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair
if "%RPAR%"=="" set RPAR=100
if "%VPAR%"=="" set VPAR=120
start "PBSD round %N% ORCHESTRATOR" cmd /k wsl -d Ubuntu -- bash -lc "export MAXREC=%MAXREC%; bash %R%/round_run.sh %N% %RB% %VB% %RPAR% %VPAR%"
timeout /t 3 /nobreak > nul
start "PBSD round %N% LOG" cmd /k wsl -d Ubuntu -- bash -lc "mkdir -p /mnt/c/ParanoidBSD/mined-data/repair%N%; touch /mnt/c/ParanoidBSD/mined-data/repair%N%/round.log; tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/repair%N%/round.log"
