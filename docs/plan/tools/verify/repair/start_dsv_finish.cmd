@echo off
rem start_dsv_finish.cmd <review-budget-usd> [parallel]   Waits for the dsv repair stage to end, then ingests (dsv_finish.sh): re-verify + review + state + export_manual + tag_final.
set VB=%1
set VP=%2
if "%VP%"=="" set VP=100
set R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair
start "PBSD dsv FINISH" cmd /k wsl -d Ubuntu -- bash -lc "bash %R%/dsv_finish.sh %VB% %VP%"
