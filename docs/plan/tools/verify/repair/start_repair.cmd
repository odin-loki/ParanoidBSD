@echo off
rem start_repair.cmd [budget-usd]   Repair every rejected function (repair/todo.tsv, 41,068 records, 81,651 functions) with GLM-5.3-Flash, in its own window.
rem Resumable: repaired records are skipped. Stop cleanly: create C:\ParanoidBSD\mined-data\verify\repair\STOP . When it finishes, the repairs are re-verified
rem (repair_verify.sh) and the finisher window commits the results.
set BUDGET=%1
if "%BUDGET%"=="" set BUDGET=40
if exist C:\ParanoidBSD\mined-data\verify\repair\STOP del C:\ParanoidBSD\mined-data\verify\repair\STOP
start "PBSD repair RUN" cmd /k wsl -d Ubuntu -- bash -lc "bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair/repair_all.sh 999999 %BUDGET% 100 && bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair/repair_verify.sh"
