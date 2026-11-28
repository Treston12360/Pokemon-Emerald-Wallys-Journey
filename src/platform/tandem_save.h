/*
 * Project Tandem S2: saves on the server (Brennan 2026-11-26 18:23: "both" - a backup of every save, and the same save
 * on any PC). docs/TANDEM_TASKS.md has the plan; tandem_net.c moves the bytes, this decides what to do with them.
 *
 * Only a game that joins the server program does any of this; anything else (playing alone, hosting, joining a
 * friend's game) is exactly as before. The whole save file is the unit.
 *   - The game remembers the last save it and the server agreed on (WallysJourney.sav.tandemsync: its hash).
 *   - At start, before the game reads the save, it waits up to 8 s for the server's answer:
 *       the same save                     -> play;
 *       only this PC's changed            -> send it;
 *       only the server's changed (or this PC has none) -> take the server's (this PC's kept as
 *                                            WallysJourney.sav.before-server-YYYYMMDD-HHMMSS);
 *       both changed                      -> ask which one to play (the other is kept).
 *   - After every save in the game (3 s after the last write) it sends the save.
 *   - Connected only later (the server was down at start): it never sends over a newer save on the server; the window
 *     title says to restart the game to choose.
 * Test aids: WJPC_TEST_SAVE_CHOICE=pc|server answers the "which one" question.
 */
#ifndef GUARD_TANDEM_SAVE_H
#define GUARD_TANDEM_SAVE_H

#include <stdbool.h>

struct SDL_Window;

/* at start, after the network started and before the game thread: TRUE if it changed the save file - the caller reads
 * it again. closeSave: closes the game's open save file (called before the file is replaced). */
bool TandemSave_BootCheck(const char *savePath, struct SDL_Window *window, void (*closeSave)(void));
/* the save file was just written (any thread) */
void TandemSave_Touched(void);
/* every frame, main thread: sends the save when it settled, takes the server's answers */
void TandemSave_Update(void);
/* closing the game: a save made in the last seconds is sent now (waits up to 5 s for the server) */
void TandemSave_Quit(void);
/* a line for the window title ("" none); *gen changes when it does */
const char *TandemSave_Note(unsigned *gen);

#endif
