#!/bin/bash
# T5 test: A (host) at Littleroot 10,12 faces B (11,12), asks to trade; B says yes; both trade their first Pokemon;
# then A cancels out of the trade screen; both back in Littleroot.
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=/home/claude/wjwork/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
EXE=${TANDEM_EXE:-/home/claude/wjwork/cx/tandem3.exe}; W3D=${W3D:-0}; TT=${TT:-/tmp/tt}; TO=${TO:-1100}
wineserver -k 2>/dev/null; wineserver -f -p 2>/dev/null & WS=$!; sleep 1
D=:178; Xvfb $D -screen 0 1920x1080x24 +extension GLX -nolisten tcp >/dev/null 2>&1 & XP=$!; sleep 2
for p in A B; do rm -rf $TT/$p; mkdir -p $TT/$p/f; ln -s /home/claude/wjwork/cx/voxel $TT/$p/voxel; cp /home/claude/wjwork/cx/bren9.sav $TT/$p/WallysJourney.sav; sed "s/world3d=1/world3d=$W3D/" /home/claude/wjwork/cx/bren7.cfg > $TT/$p/WallysJourney.cfg; done
cp ${SA:-$TT/sA5.txt} $TT/A/s.txt; cp ${SB:-$TT/sB5.txt} $TT/B/s.txt
(cd $TT/A && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_TEST_WINDOW_NOBATTLE=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=$W3D WJPC_TEST_NO_BATTLES=1 WJPC_TEST_PACE=1 WJPC_TANDEM_DEBUG=1 WJPC_TANDEM_HOST=27016 WJPC_TANDEM_NAME=Brennan timeout $TO wine $EXE > out.log 2>&1) & HA=$!
sleep 3
(cd $TT/B && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_TEST_WINDOW_NOBATTLE=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=$W3D WJPC_TEST_NO_BATTLES=1 WJPC_TEST_PACE=1 WJPC_TANDEM_DEBUG=1 WJPC_TANDEM_JOIN=127.0.0.1:27016 WJPC_TANDEM_NAME=Friend timeout $TO wine $EXE > out.log 2>&1) & HB=$!
wait $HA; sleep 2; wineserver -k 2>/dev/null; wait $HB
kill $XP $WS 2>/dev/null; wineserver -k 2>/dev/null
for p in A B; do for f in $TT/$p/f/*.png; do [ -f "$f" ] || continue; case $f in *.p.png) continue;; esac; ffmpeg -loglevel error -y -f ppm_pipe -i $f ${f%.*}.p.png 2>/dev/null && rm -f $f; done; echo "== $p"; ls $TT/$p/f | tr '\n' ' '; echo; done
