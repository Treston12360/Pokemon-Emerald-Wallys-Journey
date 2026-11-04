#!/bin/bash
# usage: capture_trainer.sh NAME MAP_CONST X Y "C statements"  (talk to the trainer above the player)
# The C statements run at new game (WJ_EXTRA), e.g. clear the leader's trainer flag.
set -u
NAME=$1 MAP=$2 X=$3 Y=$4 EXTRA=${5:-}
W=/home/claude/wj3dwork; P=/home/claude/tools-src/pret
export PATH=/opt/xpack/xpack-arm-none-eabi-gcc-14.2.1-1.1/bin:$PATH
# 1. vanilla capture build starting on this spot
sed -i "s/^#define WJ_MAP .*/#define WJ_MAP $MAP/; s/^#define WJ_X .*/#define WJ_X $X/; s/^#define WJ_Y .*/#define WJ_Y $Y/" $P/src/new_game.c
sed -i '/^#define WJ_EXTRA /d' $P/src/new_game.c
[ -n "$EXTRA" ] && sed -i "1i #define WJ_EXTRA $EXTRA" $P/src/new_game.c
touch $P/src/new_game.c
( cd $P && make -j2 modern > $W/auto/$NAME.pret.log 2>&1 ); rc=$?
sed -i '/^#define WJ_EXTRA /d' $P/src/new_game.c
[ $rc = 0 ] || { echo "$NAME: pret build failed"; exit 1; }
# 2. new game + save in mGBA
mkdir -p $W/auto/$NAME && cd $W/auto/$NAME
python3 -c "open('v.sav','wb').write(b'\xff'*131072)"
cat > save.txt <<'S'
1200
10 S
300
10 A
400
10 A
300
10 A
1200
10 S
90
10 D
30
10 D
30
10 D
30
10 D
30
10 A
120
10 A
400
10 A
1000
shot saved
S
/home/claude/tools-src/harness $P/pokeemerald_modern.gba save.txt 1793790000 v.sav > /dev/null 2>&1
python3 -c "
d=open('v.sav','rb').read(); n=sum(1 for b in d if b!=0xff); print('$NAME save bytes', n); raise SystemExit(0 if n>1000 else 1)" || { echo "$NAME: save failed"; exit 1; }
# 3. 3DS capture (3D overworld + 3D battles, OpenGL)
SD=$W/auto/$NAME/sd/Azahar/sdmc/3ds/emerald3ds
rm -rf $W/auto/$NAME/sd && mkdir -p $SD && cp ${PAK:-$W/blank2.pak} $SD/emerald3ds.pak && head -c 131072 v.sav > $SD/emerald3ds.sav
printf "voxel=1\nvoxel_battle=1\n" > $SD/settings.txt; touch $SD/debug.txt
mkdir -p shots
{ echo 1200; for i in $(seq 16); do printf "10 A\n90\n"; done; for i in $(seq 10); do printf "10 B\n60\n"; done; echo "shot shots/ow";
  printf "10 u\n60\n10 A\n120\n"; for i in $(seq 1 14); do printf "10 B\n150\n"; done
  for i in $(seq 1 6); do printf "200\nshot shots/c$i\n"; done; } > cap.txt
cd $W/auto/$NAME && FE_GL=1 FE_GFX=OpenGL timeout 3300 /home/claude/tools-src/azahar_fe /home/claude/dl/azahar/azahar_libretro.so ${GAME3DSX:-/home/claude/dl/em3ds/payload/Emerald3DS.3dsx} cap.txt $W/sys sd > cap.log 2>&1
echo "$NAME capture rc=$? shots=$(ls shots | wc -l)"
grep -a "ERROR" $SD/port.log 2>/dev/null | grep -v "ndsp\|clock\|nuzlocke" | head -3
