# Project Tandem test tools (T1, 2026-11-27, session 014PwRDj)

- `test_client.py PORT BUILD NAME [SECONDS]` - a tiny Tandem client: says hello with BUILD (the game's build, from
  `build/pc-win32/gen/tandem_build.h`, or a wrong one to test refusing), prints every message, answers pings.
- `test_three_players.sh FRAMES` - on one Linux machine with Wine and Xvfb (this container's set-up): a host
  (Brennan) and two joining games (Treston, Friend3), each in its own folder with its own settings/save, plus test
  clients: a 4th player while full (refused, "the game is full"), a wrong build (refused), a right one (joins). Prints
  each game's `WallysJourney_tandem.log`. TANDEM_EXE= the exe, TT= a scratch folder.
The protocol is described at the top of `src/platform/tandem_net.c`.
