#!/bin/bash
# kills by process NAME only (never -f: a command line naming these would kill its own shell)
pkill -9 -x wineserver 2>/dev/null
pkill -9 '\.exe$' 2>/dev/null
pkill -9 -x Xvfb 2>/dev/null
pkill -9 -x xvfb-run 2>/dev/null
sleep 2
