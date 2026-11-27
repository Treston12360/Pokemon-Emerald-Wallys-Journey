#!/bin/bash
# host + 2 game joiners + python clients (full, wrong build) on one machine
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=/home/claude/wjwork/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
EXE=${TANDEM_EXE:-/home/claude/wjwork/cx/tandem2.exe}
wineserver -k 2>/dev/null; wineserver -f -p 2>/dev/null & WS=$!; sleep 1
D=:177; Xvfb $D -screen 0 1280x800x24 +extension GLX -nolisten tcp >/dev/null 2>&1 & XP=$!; sleep 2
for p in A B C; do rm -rf ${TT:-/tmp/tt}/$p; mkdir -p ${TT:-/tmp/tt}/$p; w=$1; [ $p = A ] && w=$(( $1 * 2 )); [ $p = C ] && w=$(( $1 / 2 )); printf "wait %s\nquit\n" $w > ${TT:-/tmp/tt}/$p/s.txt; cp /home/claude/wjwork/cx/bren9.sav ${TT:-/tmp/tt}/$p/WallysJourney.sav; sed 's/world3d=1/world3d=0/' /home/claude/wjwork/cx/bren7.cfg > ${TT:-/tmp/tt}/$p/WallysJourney.cfg; done
echo "tandemHost=27015" >> ${TT:-/tmp/tt}/A/WallysJourney.cfg; echo "tandemName=Brennan" >> ${TT:-/tmp/tt}/A/WallysJourney.cfg
(cd ${TT:-/tmp/tt}/A && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_SCRIPT=s.txt timeout 300 wine $EXE > out.log 2>&1) & HA=$!
sleep 6
(cd ${TT:-/tmp/tt}/B && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_SCRIPT=s.txt WJPC_TANDEM_JOIN=127.0.0.1:27015 WJPC_TANDEM_NAME=Treston timeout 300 wine $EXE > out.log 2>&1) & HB=$!
sleep 4
(cd ${TT:-/tmp/tt}/C && DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_SCRIPT=s.txt WJPC_TANDEM_JOIN=localhost WJPC_TANDEM_NAME=Friend3 timeout 300 wine $EXE > out.log 2>&1) & HC=$!
sleep 8
B_ID="$(cut -d'"' -f2 /home/claude/wj-3d/build/pc-win32/gen/tandem_build.h)"; echo "== 4th player (python, game full):"; python3 ${TT:-/tmp/tt}/proto_client.py 27015 "$B_ID" Fourth 2
wait $HC; echo "== after C quit, wrong build (python):"; python3 ${TT:-/tmp/tt}/proto_client.py 27015 wrongbuild123 Odd 2
echo "== right build (python):"; python3 ${TT:-/tmp/tt}/proto_client.py 27015 "$(cat /home/claude/wj-3d/build/pc-win32/gen/tandem_build.h | cut -d'"' -f2)" Pyth 5
wait $HA $HB
kill $XP $WS 2>/dev/null; wineserver -k 2>/dev/null
for p in A B C; do echo "== $p log"; cat ${TT:-/tmp/tt}/$p/WallysJourney_tandem.log; grep -c "" ${TT:-/tmp/tt}/$p/WallysJourney.cfg >/dev/null; grep tandem ${TT:-/tmp/tt}/$p/WallysJourney.cfg; done
