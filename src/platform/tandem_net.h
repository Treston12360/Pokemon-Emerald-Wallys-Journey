/*
 * Project Tandem (Brennan's online multiplayer, docs/TANDEM_TASKS.md, docs/TANDEM_AUDIT.md) - T1: the network link.
 *
 * Two or three PCs running the same build connect: one hosts, the others join (Brennan 2026-11-27: "3 first";
 * connecting with "a free tool" - a LAN address, or a free private network such as Tailscale). On joining, each side
 * checks the other runs the same build (git commit) and refuses a different one. Connected, they swap small test
 * messages (hello, pings). Nothing in the game uses it yet (T2: seeing each other walk).
 *
 * Rules (TANDEM_TASKS.md): the network runs on its own thread and never makes the game wait; nothing here touches the
 * game's data or drawing; with nothing set, nothing starts - single-player exactly as before.
 *
 * Settings, in WallysJourney.cfg (or the same names as environment variables, upper case with WJPC_ in front, for
 * tests - WJPC_TANDEM_HOST etc.):
 *   tandemHost=PORT          host on PORT (e.g. tandemHost=27015)
 *   tandemJoin=ADDRESS:PORT  join the host at ADDRESS (a name or an IP, e.g. 100.64.1.2:27015)
 *   tandemName=NAME          the name friends see (up to 15 letters; default "Player")
 * What happens is written to WallysJourney_tandem.log next to the settings file, and shown in the window's title.
 */
#ifndef GUARD_TANDEM_NET_H
#define GUARD_TANDEM_NET_H

#include <stdbool.h>
#include <stdio.h>

#define TANDEM_MAX_PLAYERS 3
#define TANDEM_DEFAULT_PORT 27015

/* a line of the settings file: TRUE if it was one of Tandem's (and taken) */
bool TandemNet_ConfigLine(const char *line);
/* Tandem's lines for the settings file (only those that are set) */
void TandemNet_WriteConfig(FILE *configFile);
/* start hosting or joining if the settings ask for it (logPath: the log file); does nothing otherwise */
void TandemNet_Start(const char *logPath);
/* close every connection and stop the thread */
void TandemNet_Stop(void);
/* TRUE while hosting or joining (connected or trying) */
bool TandemNet_Active(void);
/* a short line for the window title ("" when not active); changes - compare *gen with the last one seen */
const char *TandemNet_StatusText(unsigned *gen);

/* the settings changed (the server gave this PC its key): write the settings file again; TRUE once per change */
bool TandemNet_TakeConfigChanged(void);
/* the server program (src/platform/server/tandem_server.c): hosts with no player of its own, lets new players in with
 * the invite code (each gets a key, kept in accountsPath, so the code is needed only once) */
bool TandemNet_StartServer(unsigned port, const char *invite, const char *serverName, const char *accountsPath,
                           const char *logPath);

/* T2: where a player is (src/tandem_overworld.c fills and reads these; nothing here knows what they mean) */
typedef struct
{
    unsigned char valid;        /* 0: not in the overworld (or not known yet) */
    unsigned char mapGroup, mapNum;
    short x, y;                 /* the player's tile, as the game keeps it (map coordinates + 7) */
    unsigned char facing;       /* DIR_* */
    unsigned char speed;        /* 0 walking, 1 running / Acro Bike, 2 Mach Bike, 3 surfing */
    unsigned short gfx;         /* the player's graphics id (outfit, bike, surfing) */
    unsigned char elevation;
    /* the Pokemon following the player (Brennan 2026-11-27 15:05: "we do want the pokemon to follow on everyones
     * screen"): its graphics id, 0 when none is shown; its tile and facing */
    unsigned short followerGfx;
    short followerX, followerY;
    unsigned char followerFacing;
    /* T4: the player's outfit (Wally's Journey outfits): its number, 0xFF when none; skin tone and two colours */
    unsigned char outfit, outfitSkin, outfitColour1, outfitColour2;
} TandemState;

/* my place, every overworld frame (the network sends it when it changes) */
void TandemNet_SetMyState(const TandemState *state);
/* my player number (0 = the host) */
unsigned TandemNet_MySlot(void);
/* (test aid: the wall clock in milliseconds) */
unsigned TandemNet_WallMs(void);
/* TRUE while player `slot` is connected (not me) */
bool TandemNet_SlotConnected(unsigned slot);
/* the oldest place of player `slot` not yet taken, in the order they were sent; FALSE when none is waiting */
bool TandemNet_PopState(unsigned slot, TandemState *out);

/* T4: player cards - opaque bytes to this module (src/tandem_overworld.c makes and reads them; the same build on
 * every PC, so the same layout). Mine is sent when it changes and every few seconds; a friend's latest is kept. */
#define TANDEM_CARD_MAX 200
void TandemNet_SetMyCard(const void *card, unsigned len);
/* friend `slot`'s card into `out` (at most `max` bytes); its length, 0 when none has come yet */
unsigned TandemNet_GetCard(unsigned slot, void *out, unsigned max);

/* T5: a message to one player (trades, battles), passed on by the server or host; `kind` and the bytes mean what
 * src/tandem_link.c says. Sent soon (this module's thread); FALSE if the queue is full. */
#define TANDEM_P2P_MAX 48
bool TandemNet_SendTo(unsigned slot, unsigned kind, const void *data, unsigned len);
/* the oldest message that came for me: who from, its kind, its bytes (at most `max`); its length, or -1 when none */
int TandemNet_PopP2P(unsigned *fromSlot, unsigned *kind, void *data, unsigned max);

/* S2: saves on the server (src/platform/tandem_save.c decides what to do; this only moves the bytes). Only a game that
 * joins the server program has them - a friend's hosting game keeps no saves. The whole save file is sent, in parts. */
#define TANDEM_SAVE_MAX (8u * 1024 * 1024)
typedef struct
{
    int kind;                    /* 0: not connected (yet); 1: joined a friend's game (no saves there); 2: the server */
    int has;                     /* (server) it has a save of mine */
    unsigned long long hash;     /* its hash (TandemNet_Hash), size and when it was saved (seconds since 1970) */
    unsigned size;
    long long savedAt;
    unsigned connection;         /* goes up with every new connection to the server */
} TandemServerSave;
/* the server's save, as far as this game knows */
void TandemNet_GetServerSave(TandemServerSave *out);
/* the 64-bit hash both sides use (FNV-1a) */
unsigned long long TandemNet_Hash(const void *data, unsigned size);
/* fetch my save from the server; then TandemNet_SaveGetResult: 0 still coming, 1 here (*data: malloc'd, free it),
 * -1 failed (none there, or the connection broke) */
bool TandemNet_SaveGet(void);
int TandemNet_SaveGetResult(unsigned char **data, unsigned *size);
/* send my save to the server (copied); `base`: the hash of the save this game knows the server has (0: none) - the
 * server refuses it if its own save is a different one by then. Then TandemNet_SavePutResult: 0 still going (or none),
 * 1 stored (*hash: which), -1 refused or failed */
bool TandemNet_SavePut(const void *data, unsigned size, long long savedAt, unsigned long long base);
int TandemNet_SavePutResult(unsigned long long *hash);
/* the server program: where it keeps everyone's saves (a folder; made if missing) */
void TandemNet_SetServerSaves(const char *dir);
/* TRUE when this game joins (a server or a friend's game) */
bool TandemNet_IsJoining(void);
/* a line in Tandem's log (WallysJourney_tandem.log) */
void TandemNet_Log(const char *fmt, ...);

#endif
