#!/bin/bash
# Project Tandem S2 test: saves on the server (docs/TANDEM_TASKS.md S2; src/platform/tandem_save.h).
# One server (WallysJourneyServer.exe, Wine) and two "PCs" of the same player (P1, P2: folders; P2 has P1's settings
# file, so the same key). Each step runs one game at a time, checks the files, and prints PASS/FAIL.
#   1  P1 (a save, server has none)        -> sends it
#   2  P1 saves in the game                -> sent; the server keeps the one before by date
#   3  P2 (no save)                        -> takes the server's
#   4  P1 and P2 both play without the server (each saves)
#   5  P1 with the server (only P1 changed) -> sends it
#   6  P2 with the server (both changed), answers "the server's" -> takes it, its own kept beside it
#   7  P2 changes offline, P1 sends another; P2 with the server answers "this PC's" -> the server takes P2's
#   8  P1 starts with the server down, it comes up later; the server's changed (step 7) -> P1 doesn't send its save
#   9  P1 with the server from the start (both changed), answers "this PC's" -> the server takes it
#  10  P1 saves and closes the game at once (no time to settle) -> still sent as it closes
# Usage: tools/tandem/s2/run_s2.sh   (after make -f Makefile.pc WIN=1; copies the exe and the server from the repo)
# Needs: Wine (wine-10.0), Xvfb, the test save bren9.sav and settings bren7.cfg in /home/claude/wjwork/cx (see
# tools/tandem/t5/README.md), the fast-boot script /tmp/tt/fb_t6b.txt (head -14 of a T6 script: Continue).
export PATH=/opt/wine-dl/wine-10.0-amd64-wow64/bin:$PATH WINEPREFIX=/home/claude/wjwork/wpfx WINEDEBUG=-all WINEDLLOVERRIDES="mscoree,mshtml="
REPO=$(cd "$(dirname "$0")/../../.." && pwd)
CX=/home/claude/wjwork/cx; TT=${TT:-/tmp/s2}; FB=${FB:-/tmp/tt/fb_t6b.txt}
EXE=$TT/WallysJourney.exe; SRV=$TT/WallysJourneyServer.exe
PORT=27031
rm -rf $TT; mkdir -p $TT/S $TT/P1 $TT/P2
cp $REPO/WallysJourney.exe $EXE; cp $REPO/WallysJourneyServer.exe $SRV
wineserver -k 2>/dev/null; wineserver -f -p 2>/dev/null & WS=$!; sleep 1
D=:179; Xvfb $D -screen 0 1920x1080x24 +extension GLX -nolisten tcp >/dev/null 2>&1 & XP=$!; sleep 2
printf 'port=%s\ninvite=TEST42\nname=Test server\n' $PORT > $TT/S/WallysJourneyServer.cfg

startServer() { (cd $TT/S && exec wine $SRV >> out.log 2>&1) & HS=$!; sleep 4; }
stopServer() { kill $HS 2>/dev/null; pkill -f WallysJourneyServer.exe; sleep 2; }
# play PC STEP [online|offline] [money] [choice]: fast-boot, money set + saved (if given), wait, quit
play() {
    local pc=$1 step=$2 net=$3 money=$4 choice=$5 dir=$TT/$1
    cp $FB $dir/s.txt
    echo "wait 120" >> $dir/s.txt
    echo "state money 0" >> $dir/s.txt
    if [ -n "$money" ]; then echo "state money 0 $money" >> $dir/s.txt; echo "state save 0" >> $dir/s.txt; fi
    echo "wait ${WAITF:-700}" >> $dir/s.txt
    echo "quit" >> $dir/s.txt
    local join=127.0.0.1:$PORT; [ "$net" = offline ] && join=127.0.0.1:27099
    sed -i "s/^tandemJoin=.*/tandemJoin=$join/" $dir/WallysJourney.cfg
    (cd $dir && env DISPLAY=$D WJPC_TEST_WINDOW=1 WJPC_WINDOW=1280x720 WJPC_SCRIPT=s.txt WJPC_3D=0 WJPC_TEST_NO_BATTLES=1 \
        WJPC_TEST_PACE=1 ${choice:+WJPC_TEST_SAVE_CHOICE=$choice} timeout 300 wine $EXE > out_$step.log 2>&1)
    cp $dir/WallysJourney_tandem.log $dir/tandem_$step.log 2>/dev/null
}
KEYF() { grep '^tandemKey=' $TT/P1/WallysJourney.cfg | cut -d= -f2 | tr -d '\r'; }
srvsave() { echo $TT/S/WallysJourneyServer_saves/$(KEYF).sav; }
same() { cmp -s "$1" "$2"; }
check() { if eval "$2"; then echo "PASS $1"; else echo "FAIL $1"; FAILS=$((FAILS+1)); fi; }
money() { grep "state money" $TT/$1/state.log | tail -n 1 | awk '{print $NF}' | tr -d '\r'; }
FAILS=0

# P1: the test save; P2: none, and P1's settings (after step 1, for the key)
cp $CX/bren9.sav $TT/P1/WallysJourney.sav
for p in P1 P2; do ln -s $CX/voxel $TT/$p/voxel; sed "s/world3d=1/world3d=0/" $CX/bren7.cfg > $TT/$p/WallysJourney.cfg
    printf 'tandemJoin=127.0.0.1:%s\ntandemName=Brennan\ntandemInvite=TEST42\n' $PORT >> $TT/$p/WallysJourney.cfg; done

startServer
play P1 1 online
check "1 the server has P1's save" 'same $TT/P1/WallysJourney.sav $(srvsave)'
check "1 P1 remembers it" '[ -f $TT/P1/WallysJourney.sav.tandemsync ]'

play P1 2 online 111111
check "2 the server has P1's new save" 'same $TT/P1/WallysJourney.sav $(srvsave)'
check "2 the one before kept by date" '[ $(ls $TT/S/WallysJourneyServer_saves/ | grep -c "^$(KEYF)\.[0-9]") -ge 1 ]'

cp $TT/P1/WallysJourney.cfg $TT/P2/WallysJourney.cfg
play P2 3 online
check "3 P2 took the server's save" 'same $TT/P2/WallysJourney.sav $(srvsave)'
check "3 P2 plays it (money 111111)" '[ "$(money P2)" = 111111 ]'

play P1 4a offline 222222
play P2 4b offline 333333
check "4 nothing sent while offline" '! same $TT/P1/WallysJourney.sav $(srvsave) && ! same $TT/P2/WallysJourney.sav $(srvsave)'

play P1 5 online
check "5 the server has P1's offline save" 'same $TT/P1/WallysJourney.sav $(srvsave)'

cp $TT/P2/WallysJourney.sav $TT/p2_before6.sav
play P2 6 online "" server
check "6 P2 took the server's (asked: server)" 'same $TT/P2/WallysJourney.sav $(srvsave)'
check "6 P2 plays it (money 222222)" '[ "$(money P2)" = 222222 ]'
check "6 P2's own kept beside it" 'ls $TT/P2/WallysJourney.sav.before-server-* >/dev/null 2>&1 && same $TT/p2_before6.sav $(ls $TT/P2/WallysJourney.sav.before-server-* | tail -n 1)'

play P2 7a offline 444444
play P1 7b online 555555
check "7 the server has P1's 555555" 'same $TT/P1/WallysJourney.sav $(srvsave)'
play P2 7c online "" pc
check "7 P2 asked: this PC's - the server took it" 'same $TT/P2/WallysJourney.sav $(srvsave)'
check "7 P2 kept playing its own (money 444444)" '[ "$(money P2)" = 444444 ]'

stopServer
cp $(srvsave) $TT/srv_before8.sav
(sleep 25; cd $TT/S && exec wine $SRV >> out.log 2>&1) &
WAITF=2400 play P1 8 online 666666
check "8 the server kept P2's save (P1 didn't send over it)" 'same $TT/srv_before8.sav $(srvsave)'
check "8 P1 was told (log)" 'grep -q "restart the game to choose" $TT/P1/tandem_8.log'

play P1 9 online "" pc
check "9 P1 asked: this PC's - the server took it (666666)" 'same $TT/P1/WallysJourney.sav $(srvsave) && [ "$(money P1)" = 666666 ]'

WAITF=20 play P1 10 online 777777
check "10 saved and closed at once - the server has it" 'same $TT/P1/WallysJourney.sav $(srvsave)'

stopServer; kill $XP $WS 2>/dev/null; wineserver -k 2>/dev/null
echo "== $FAILS failed"; ls -la $TT/S/WallysJourneyServer_saves/ | sed 's/[0-9a-f]\{32\}/KEY/'
