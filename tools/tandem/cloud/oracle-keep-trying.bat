@echo off
rem Wally's Journey (Project Tandem): keeps asking Oracle for the free server computer - oracle-keep-trying.ps1
rem (docs/TANDEM_ORACLE_SETUP.md, "Let a script keep trying"). Close the window to stop it.
rem (-WaitSeconds: how far apart the tries are - Brennan 2026-11-27 20:36 asked about every 30 seconds)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0oracle-keep-trying.ps1" -WaitSeconds 30
