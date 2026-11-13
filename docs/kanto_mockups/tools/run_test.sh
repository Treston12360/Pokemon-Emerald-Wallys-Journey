#!/bin/bash
# usage: run.sh DIR SAVE [env...]   (DIR has s.txt; shots -> png)
D=$1; SAVE=$2; shift 2
cd $D && cp $SAVE g.sav && rm -f *.ppm *.png WallysJourney_errors.txt
env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy WJPC_SCRIPT=s.txt WJPC_SAVE=g.sav "$@" timeout -s KILL 600 ${WJBIN:-/home/claude/pokemon-emerald-wallys-journey/wallys-journey} > out.log 2>&1
echo "exit $?"; grep -c "skipped" out.log; [ -f WallysJourney_errors.txt ] && head -5 WallysJourney_errors.txt
for f in *.ppm; do [ -f "$f" ] && python3 -c "from PIL import Image; import sys; Image.open('$f').save('${f%.ppm}.png')"; done
