#!/bin/sh
# Build the complete Windows game folder + zip (Project Deluxe / ARIA / Polish all use this).
# Owner rule (2026-11-13): every build sent to Treston or Brennan is the WHOLE game folder - exe, DLLs,
# voxel/ data, README, launchers - so they only drop in their own settings file (WallysJourney.cfg) and
# save (WallysJourney.sav). Never send a .gba, never send the exe alone.
#   usage: tools/pc/make_package.sh NAME [OUTDIR]     (run from the repo root, after make -f Makefile.pc WIN=1)
set -e
NAME=${1:?name, e.g. WallysJourney-2026-11-13}
OUT=${2:-build/packages}
DEPS=build/pc-deps/SDL2/i686-w64-mingw32/bin
D="$OUT/$NAME"
[ -f WallysJourney.exe ] || { echo "no WallysJourney.exe: run make -f Makefile.pc WIN=1 first"; exit 1; }
rm -rf "$D" "$OUT/$NAME.zip"; mkdir -p "$D"
cp WallysJourney.exe "$D/"; i686-w64-mingw32-strip "$D/WallysJourney.exe"
cp "$DEPS/SDL2.dll" "$DEPS/SDL2_image.dll" "$D/"
cp -r pc_data/voxel "$D/voxel"        # the 3D data (regenerate with tools/voxel3ds/README.md, then copy here)
printf '@echo off\r\ncd /d "%%~dp0"\r\nset WJPC_3D=1\r\nstart "" WallysJourney.exe\r\n' > "$D/Play in 3D.bat"
cat > "$D/README.txt" <<'R'
Pokemon Emerald: Wally's Journey (Windows)

WallysJourney.exe = the game. 3D World, 3D Battles and Gamma Lighting are in Options > Settings > Video.
"Play in 3D.bat" = start straight in 3D.
Keep the voxel folder next to the exe (it is the 3D world).
Your files: copy your WallysJourney.cfg (settings) and WallysJourney.sav (save) into this folder.
Old GBA .sav files import: name it WallysJourney.sav and put it next to the exe.
R
(cd "$OUT" && zip -qr "$NAME.zip" "$NAME")
for f in WallysJourney.exe SDL2.dll SDL2_image.dll voxel/buildings.bin voxel/relief.bin voxel/regions.bin \
         voxel/signposts.bin voxel/trees.rgba5551 voxel/gamma_clouds.bin README.txt; do
    [ -e "$D/$f" ] || { echo "MISSING $f"; exit 1; }
done
echo "$OUT/$NAME.zip"
