@echo off
rem start_repair.cmd [budget-usd]   Repair every rejected function (repair/todo.tsv) with GLM-5.3-Flash, supervised by repair_watch.sh (restart on death or stall,
rem stop on a real API credit/auth error, hourly restart cap). Resumable. Stop cleanly: create C:\ParanoidBSD\mined-data\verify\repair\STOP .
rem When the repair finishes (DONE file), the repairs are re-verified (repair_verify.sh); the finisher window commits the results.
set BUDGET=%1
if "%BUDGET%"=="" set BUDGET=40
if exist C:\ParanoidBSD\mined-data\verify\repair\STOP del C:\ParanoidBSD\mined-data\verify\repair\STOP
set R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair
start "PBSD repair WATCHER" cmd /k wsl -d Ubuntu -- bash -lc "bash %R%/repair_watch.sh %BUDGET% 100; [ -f /mnt/c/ParanoidBSD/mined-data/verify/repair/DONE ] && bash %R%/repair_verify.sh"
timeout /t 3 /nobreak > nul
start "PBSD repair LOG" cmd /k wsl -d Ubuntu -- bash -lc "tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/verify/repair/repair.log /mnt/c/ParanoidBSD/mined-data/verify/repair/watch.log"
