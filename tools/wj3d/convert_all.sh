#!/bin/bash
# convert every captured map listed in routes.txt (or given names); SHADOW_<name>=blur picks blur
cd /home/claude/wj3dwork/auto
names=${@:-$(awk '{print $1}' routes.txt)}
for n in $names; do
  [ -f $n/shots/c6.ppm ] || { echo "$n: no capture"; continue; }
  v=SHADOW_$n; SHADOW=${!v:-} python3 autoclean.py $n/shots/c6.ppm $n/clean400.png && python3 to_gba.py $n/clean400.png $n/gba.png \
   && (cd /home/claude/pokemon-emerald-wallys-journey && python3 tools/wj3d_backdrop.py /home/claude/wj3dwork/auto/$n/gba.png graphics/battle_environment/wj3d/$n | tail -1)
done
