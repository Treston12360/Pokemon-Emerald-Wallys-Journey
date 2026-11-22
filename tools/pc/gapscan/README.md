# Gap scan (magenta test at full resolution)

Brennan 2026-11-21 21:53: run gap tests at 1920x1080 and 2560x1440, his resolutions.

`gapscan.sh EXE NAME BODY WxH [ENV=value]...` runs a test-script BODY (lines `warp G N X Y`, `wait F`,
`winshot f/NAME/<shot>.png`, `wait 3`) in batches of 20 warps with F7 on (magenta wherever nothing is drawn), using
the Polish test harness `runf.sh` (Wine + Xvfb; see docs/PROJECT_PC_PORT.md). After each batch `gapscan.py` writes
`<shot> <magenta pixels> <box>` to NAME.cnt, keeps a 960-wide jpg in f/NAME_jpg/ and deletes the raw shots (6-11 MB
each, the disk filled at 200 shots). Compare two runs with `join`:

    join <(sort a.cnt|awk '{print $1,$2}') <(sort b.cnt|awk '{print $1,$2}') | awk '$2!=$3'

Give a map time to load (wait 150+): a neighbour not yet loaded shows as a big magenta block (Pacifidlog, 21q).
The paths inside are the test machine's; change the cd line for yours.
