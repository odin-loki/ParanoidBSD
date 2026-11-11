@echo off
rem finish_all.cmd   After step 4 and the repair have finished: re-tag all functions + copy results (window 1), re-verify the repairs (window 2). Both resume/idempotent.
set V=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify
start "PBSD step4 TAGS" cmd /k wsl -d Ubuntu -- bash -lc "bash %V%/tag_all.sh && bash %V%/copy_results.sh; echo TAGS-DONE"
start "PBSD repair VERIFY" cmd /k wsl -d Ubuntu -- bash -lc "bash %V%/repair/repair_verify.sh; echo VERIFY-DONE"
