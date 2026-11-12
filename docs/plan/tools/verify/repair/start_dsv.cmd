@echo off
rem start_dsv.cmd <repair-budget-usd> [parallel]   morph-dsv41flash-1017 repair of the manual queue (dsv_watch.sh -> dsv_all.sh). Resumable: run it again after a stop.
rem Data: C:\ParanoidBSD\mined-data\dsv (dsv.jsonl = results, ledger.txt, watch.log). Stop cleanly: create C:\ParanoidBSD\mined-data\dsv\STOP
set RB=%1
set P=%2
if "%P%"=="" set P=64
set R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair
start "PBSD dsv repair WATCHER" cmd /k wsl -d Ubuntu -- bash -lc "export MAXN=%MAXN% DSV_EFFORT=%DSV_EFFORT%; rm -f /mnt/c/ParanoidBSD/mined-data/dsv/STOP; bash %R%/dsv_watch.sh %RB% %P%"
timeout /t 3 /nobreak > nul
start "PBSD dsv repair LOG" cmd /k wsl -d Ubuntu -- bash -lc "touch /mnt/c/ParanoidBSD/mined-data/dsv/repair.log; tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/dsv/repair.log"
