#!/bin/bash
cd /tmp/claude-0/-home-claude/3dd33a7e-e6c0-5d8c-8a91-9dc994dcd499/scratchpad/cx
# make every picture folder the body writes to, before the game starts (Brennan 2026-11-19 12:37)
for d in $(awk '$1=="winshot"{print $2}' "$3" | xargs -r -n1 dirname | sort -u); do mkdir -p "$d"; done
export WJPC_WINDOW=1280x720 WJPC_TEST_WINDOW=1
./clean.sh
EXE=$1.exe SAV=${SAVF:-bren3.sav} CFG=bren2r0.cfg W3D=1 ./uif.sh $2 $3 >/dev/null 2>&1; pkill -9 -x wineserver
for p in f/$2/*.png; do case $p in *.p.png) ;; *) ffmpeg -loglevel error -y -f ppm_pipe -i $p ${p%.png}.p.png;; esac; done; ls f/$2
