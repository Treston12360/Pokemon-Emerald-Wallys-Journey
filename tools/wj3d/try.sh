#!/bin/bash
# usage: try.sh REGEX name  -> builds pak, boots 1500 frames, prints whether screen is non-black
S=/tmp/claude-0/-home-claude/c01f291d-beb5-5ce1-b8e2-6743db5de3e3/scratchpad
cd $S/bb && BLANK_RE="$1" python3 -I mkpak_clean.py ../e3ds5/web/python ../pret/clean.gba t_$2.pak 2>&1 | grep -E "blanked|rror"
cd $S/azahar && rm -rf sv_$2 && cp -r save5 sv_$2 && cp $S/bb/t_$2.pak sv_$2/Azahar/sdmc/3ds/emerald3ds/emerald3ds.pak && rm $S/bb/t_$2.pak
printf "1500\nshot tt_$2\n" > st_$2.txt
start=$(date +%s); timeout 900 ./fe core/azahar_libretro.so game5.3dsx st_$2.txt sys sv_$2 > lg_$2.txt 2>&1; echo "secs $(( $(date +%s)-start ))"
python3 -c "
from PIL import Image; im=Image.open('tt_$2.ppm'); print('$2', 'max', max(max(p) for p in im.getdata()))"
rm -rf sv_$2
