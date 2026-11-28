#ifndef GUARD_TANDEM_LINK_H
#define GUARD_TANDEM_LINK_H

/* Project Tandem T5 (src/tandem_link.c): the game's link cable over the network, between two friends - so the game's
 * own trade screen (and later its battles) runs as it did over a cable. */

/* what asking a friend is for (the ask message's byte; data/scripts/tandem.inc uses the numbers) */
#define TANDEM_ACTIVITY_TRADE   0
#define TANDEM_ACTIVITY_SINGLE  1
#define TANDEM_ACTIVITY_DOUBLE  2
#define TANDEM_ACTIVITY_COUNT   3

/* a link with a friend is set up (from asking to trade until back in the overworld) */
bool8 TandemLink_InSession(void);
/* the session is over (the trade screen closed its link and the game is going back to the overworld) */
void TandemLink_EndSession(void);
/* once an overworld frame (TandemOverworld_Update): a friend asking to trade */
/* free in the overworld - no script, no menu, standing still (when a friend's question can come up) */
bool8 TandemLink_PlayerFree(void);
void TandemLink_OverworldUpdate(void);
/* link.c's error screen, when the link with a friend broke: its text, and A on it */
extern const u8 gText_TandemLinkLost[];
extern const u8 gText_TandemABtnContinue[];
void TandemLink_LeaveAfterError(void);
/* trade.c: the Pokemon were swapped and not saved yet (TRUE), or saved (FALSE) */
void TandemLink_SetTradeUnsaved(bool8 unsaved);
/* link.c: TRUE once a session, at its first connection - the friend's details are kept then (a Cable Club's desk
 * does that before its trade room) */
bool8 TandemLink_FirstPlayers(void);
/* link.c: the game has the link open */
bool8 Link_PCIsOpen(void);

/* specials (data/specials.inc, data/scripts/tandem.inc) */
void Tandem_Ask(void);
void Tandem_WaitAnswer(void);
void Tandem_Answer(void);
void Tandem_WaitGo(void);
void Tandem_Start(void);
void Tandem_BufferAsker(void);
void Tandem_CanBattle(void);

#endif
