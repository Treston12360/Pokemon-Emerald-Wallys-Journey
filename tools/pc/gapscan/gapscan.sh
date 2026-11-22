#!/bin/bash
# gapscan.sh EXE NAME BODY WxH [ENV=..]... : runs BODY's warps in batches of 20 (disk), counting F7 magenta per shot
cd /tmp/claude-0/-home-claude/3dd33a7e-e6c0-5d8c-8a91-9dc994dcd499/scratchpad/cx
EXE=$1; NAME=$2; BODY=$3; RES=$4; shift 4
rm -f $NAME.cnt; mkdir -p f/$NAME
awk '/^warp/{n++} {print > sprintf("'$NAME'.chunk%03d", int((n-1)/20))}' <(grep -v '^fxkey' $BODY)
for c in $NAME.chunk*; do
  { echo "fxkey 7"; cat $c; } > $c.b
  env SAVF=bren9.sav CFG=bren7.cfg RESSED='s/x/x/' WJPC_3D_NO_WEATHER=1 WJPC_RTC_EPOCH=1795089600 WJPC_WINDOW=$RES "$@" timeout 900 ./runf.sh $EXE $NAME $c.b >/dev/null 2>&1
  python3 gapscan.py $NAME.cnt f/$NAME
  rm -f $c $c.b
done
echo scanned $(wc -l < $NAME.cnt)
