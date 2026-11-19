# 3D audit test scripts (Project Polish, session 014PwRDj)

Used for the Hoenn 3D audit batches (docs/POLISH_TASKS.md). They run the Windows exe under wine on a headless X
display, warp round maps and save screenshots. Paths at the top point at the session's scratch folder (`cx`); change
them for your machine. The save (`bren3.sav`) and settings (`bren2r0.cfg`) are Brennan's and are NOT in git.

- `runf.sh EXE NAME BODY`: runs `EXE.exe` with the script BODY (lines: `warp GROUP NUM X Y`, `wait FRAMES`,
  `winshot PATH.png`). Log: `NAME.log`. **It makes every `winshot` folder first** (Brennan 2026-11-19 12:37: "make a
  pictures folder first next time before starting a batch so we dont have to restart a whole batch because of this").
- End a body with `wait 60` after the last `winshot`, or the last picture is lost.
- `clean.sh`: kills the game / wine / X by name (not `pkill -f`, which kills its own shell).
- Env: `WJPC_TEST_3D_ZOOM=28` (camera further out, for audit pictures), `WJPC_FIELD_DUMP=1` (field heights to the log).
- About 5 warps per run fit in a 1500 s timeout. Screenshots are raw PPM despite `.png`; convert with PIL.
