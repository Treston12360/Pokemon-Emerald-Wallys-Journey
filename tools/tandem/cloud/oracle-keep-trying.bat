@echo off
rem Wally's Journey (Project Tandem): keeps asking Oracle for the free server computer - oracle-keep-trying.ps1
rem (docs/TANDEM_ORACLE_SETUP.md, "Let a script keep trying"). Close the window to stop it.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0oracle-keep-trying.ps1"
