#!/bin/bash
# T5 through the server program: WallysJourneyServer.exe first; A and B join it (invite code); the trade scripts
# (sA5.txt / sB5.txt) as in run_t5.sh.
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=/home/claude/wjwork/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
EXE=/home/claude/wjwork/cx/tandem3.exe; SRV=/home/claude/wjwork/cx/WallysJourneyServer.exe; W3D=${W3D:-0}; TT=/tmp/tt; TO=${TO:-1100}
wineserver -k 2>/dev/null; wineserver -f -p 2>/dev/null & WS=$!; sleep 1
D=:178; Xvfb $D -screen 0 1920x1080x24 +extension GLX -nolisten tcp >/dev/null 2>&1 & XP=$!; sleep 2
rm -rf $TT/S; mkdir -p $TT/S; printf 'port=27016\ninvite=TEST42\nname=Test server\n' > $TT/S/WallysJourneyServer.cfg
(cd $TT/S && timeout $TO wine $SRV > out.log 2>&1) & HS=$!
sleep 5
for p in A B; do rm -rf $TT/$p; mkdir -p $TT/$p/f; ln -s /home/claude/wjwork/cx/voxel $TT/$p/voxel; cp /home/claude/wjwork/cx/bren9.sav $TT/$p/WallysJourney.sav; sed "s/world3d=1/world3d=$W3D/" /home/claude/wjwork/cx/bren7.cfg > $TT/$p/WallysJourney.cfg; done
printf 'tandemJoin=127.0.0.1:27016\ntandemName=Brennan\ntandemInvite=TEST42\n' >> $TT/A/WallysJourney.cfg
printf 'tandemJoin=127.0.0.1:27016\ntandemName=Friend\ntandemInvite=TEST42\n' >> $TT/B/WallysJourney.cfg
cp ${SA:-$TT/sA5.txt} $TT/A/s.txt; cp ${SB:-$TT/sB5.txt} $TT/B/s.txt
(cd $TT/A && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=$W3D WJPC_TEST_NO_BATTLES=1 WJPC_TEST_PACE=1 WJPC_TANDEM_DEBUG=1 timeout $TO wine $EXE > out.log 2>&1) & HA=$!
sleep 3
(cd $TT/B && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=$W3D WJPC_TEST_NO_BATTLES=1 WJPC_TEST_PACE=1 WJPC_TANDEM_DEBUG=1 timeout $TO wine $EXE > out.log 2>&1) & HB=$!
wait $HA; sleep 2; wineserver -k 2>/dev/null; wait $HB $HS
kill $XP $WS 2>/dev/null; wineserver -k 2>/dev/null
for p in A B; do for f in $TT/$p/f/*.png; do [ -f "$f" ] || continue; case $f in *.p.png) continue;; esac; ffmpeg -loglevel error -y -f ppm_pipe -i $f ${f%.*}.p.png 2>/dev/null && rm -f $f; done; echo "== $p"; ls $TT/$p/f | tr '\n' ' '; echo; done
