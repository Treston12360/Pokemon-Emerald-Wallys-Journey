#!/bin/bash
# one game, no Tandem: SAV=save SCRIPT=script W3D=0/1 OUT=dir
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=/home/claude/wjwork/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
EXE=${TANDEM_EXE:-/home/claude/wjwork/cx/tandem3.exe}; W3D=${W3D:-0}; O=${OUT:-/tmp/tt/R}
wineserver -k 2>/dev/null; wineserver -f -p 2>/dev/null & WS=$!; sleep 1
D=${DISP:-:179}; Xvfb $D -screen 0 1920x1080x24 +extension GLX -nolisten tcp >/dev/null 2>&1 & XP=$!; sleep 2
rm -rf $O; mkdir -p $O/f; ln -s /home/claude/wjwork/cx/voxel $O/voxel; cp $SAV $O/WallysJourney.sav; sed "s/world3d=1/world3d=$W3D/" /home/claude/wjwork/cx/bren7.cfg > $O/WallysJourney.cfg; cp $SCRIPT $O/s.txt
(cd $O && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=$W3D WJPC_TEST_NO_BATTLES=1 WJPC_TEST_PACE=1 timeout ${TO:-600} wine $EXE > out.log 2>&1)
kill $XP $WS 2>/dev/null; wineserver -k 2>/dev/null
for f in $O/f/*.png; do case $f in *.p.png) continue;; esac; ffmpeg -loglevel error -y -f ppm_pipe -i $f ${f%.*}.p.png 2>/dev/null && rm -f $f; done; ls $O/f
