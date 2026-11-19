#!/bin/bash
# usage: ui.sh NAME BODYFILE [3D] - his save, 2D by default; body = script lines after Continue; shots go to f/NAME
SP=/tmp/claude-0/-home-claude/3dd33a7e-e6c0-5d8c-8a91-9dc994dcd499/scratchpad
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=$SP/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
cd $SP/cx; rm -rf f/$1; mkdir -p f/$1; cp ${SAV:-bren2.sav} WallysJourney.sav; sed -e "s/world3d=1/world3d=${W3D:-0}/" -e 's/resolution=4/resolution=0/' ${CFG:-bren2.cfg} > WallysJourney.cfg
{ cat ../w3d/fastboot.txt; echo "wait 200"; sed "s#SHOT #shot f/$1/#" $2; echo quit; } > $1.txt
WJPC_TEST_NO_BATTLES=1 WJPC_SCRIPT=$1.txt WJPC_3D=${W3D:-0} timeout 1500 xvfb-run -a -s "-screen 0 1280x800x24 +extension GLX" wine ${EXE:-WallysJourney.exe} > $1.log 2>&1
for p in f/$1/*.ppm; do ffmpeg -loglevel error -y -i $p ${p%.ppm}.png && rm $p; done; ls f/$1
