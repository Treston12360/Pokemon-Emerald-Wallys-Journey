# Project Tandem test tools (T1, 2026-11-27, session 014PwRDj)

- `test_client.py PORT BUILD NAME [SECONDS]` - a tiny Tandem client: says hello with BUILD (the game's build, from
  `build/pc-win32/gen/tandem_build.h`, or a wrong one to test refusing), prints every message, answers pings.
- `test_three_players.sh FRAMES` - on one Linux machine with Wine and Xvfb (this container's set-up): a host
  (Brennan) and two joining games (Treston, Friend3), each in its own folder with its own settings/save, plus test
  clients: a 4th player while full (refused, "the game is full"), a wrong build (refused), a right one (joins). Prints
  each game's `WallysJourney_tandem.log`. TANDEM_EXE= the exe, TT= a scratch folder.
- `s2/run_s2.sh` - S2, saves on the server (2026-11-28): the server program and two "PCs" of one player (same key),
  one game at a time - first send, a save in the game sent (the one before kept by date), a new PC taking the
  server's, both played offline, "which save?" answered both ways (`WJPC_TEST_SAVE_CHOICE=pc|server`), and the server
  coming up late with a newer save (not sent over). Prints PASS/FAIL per check. Test aid: `state money 0 VALUE` sets
  the money (tells the saves apart).
The protocol is described at the top of `src/platform/tandem_net.c`.
