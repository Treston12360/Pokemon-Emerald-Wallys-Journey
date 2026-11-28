# Project Tandem T5 tests (trading)

Two Windows games under Wine on one Linux machine, a virtual screen (Xvfb), test scripts driving both.
Paths inside the runners are this session's (`/home/claude/wjwork/cx/...`: the built exe, `bren7.cfg`, `bren9.sav`,
the `voxel` 3D data, Wine in `/opt/wine-dl`) - change them to yours.

- `make_scripts.py FASTBOOT.txt DIR` - writes the scripts: `trade_A/B` (a full trade, again, both cancel, both walk),
  `edge_A/B` (No; stop waiting; the friend quits in the trade screen), `battle_A/B` (T6a), `teamup_A/B` (T6b: partners,
  then a team-up against Bug Catcher Rick on Route 102, the Exp. after it - run with run_t6.sh).
- `run_t5.sh` - A hosts, B joins: `SA=DIR/trade_A.txt SB=DIR/trade_B.txt W3D=0|1 TO=seconds ./run_t5.sh`.
  Pictures in `/tmp/tt/A/f`, `/tmp/tt/B/f`; `WJPC_TANDEM_DEBUG=1` puts the link's log in `out.log`.
- `run_t6.sh` - as run_t5.sh, but battles aren't drawn every frame (WJPC_TEST_WINDOW_NOBATTLE): for `battle_A/B`
  (T6a: a Single Battle, a hit, A gives up; `state.log` shows the HP put back as before).
- `run_t5srv.sh` - the same through `WallysJourneyServer.exe` (both join it with an invite code).
- `run_one.sh` - one game, no friend: `SAV=... SCRIPT=... W3D=... OUT=... ./run_one.sh` (e.g. a script with
  `state party 0`..`5` writes the party's species to `state.log` - to check a save after a trade).

Test aids in the game: `WJPC_TEST_PACE=1` (test scripts run at 60 frames a second, as real play - two linked games
need it), the script commands `waitfor locked|free|link|nolink|action MAX`, `state party SLOT`, `state hp SLOT [VALUE]`, `state level|exp SLOT`, `state move N`;
`WJPC_TEST_WINDOW_NOBATTLE=1`. Battle timing in scripts is touchy: `waitfor action` is reliable for the first turn
only - later turns use fixed waits.
A script's last `winshot` needs a `wait` after it before `quit`, or the picture isn't taken.
