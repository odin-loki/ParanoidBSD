@echo off
rem start_step4.cmd [budget-usd]   Step 4 of VERIFICATION_PLAN.md (DeepSeek V4.1 Flash 1017 cross-check) in its own windows.
rem   "PBSD step4 WATCHER"  starts and supervises the run (restart on stall or death, stop on a credit error); holds the WSL session open
rem   "PBSD step4 LOG"      live view of the logs
rem Data: C:\ParanoidBSD\mined-data\verify\xcheck   (not temp). Stop cleanly: create the file xcheck\STOP. Resume: delete STOP and run this again.
set BUDGET=%1
if "%BUDGET%"=="" set BUDGET=60
if exist C:\ParanoidBSD\mined-data\verify\xcheck\STOP del C:\ParanoidBSD\mined-data\verify\xcheck\STOP
start "PBSD step4 WATCHER" cmd /k wsl -d Ubuntu -- bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/step4_watch.sh %BUDGET% 160
timeout /t 5 /nobreak > nul
start "PBSD step4 LOG" cmd /k wsl -d Ubuntu -- bash -lc "tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/verify/xcheck/step4.log /mnt/c/ParanoidBSD/mined-data/verify/xcheck/watch.log"
