/*
 * Project Tandem T5 (docs/TANDEM_TASKS.md): the game's link cable, over the network, between two friends.
 *
 * Over a cable, a game sends 8 numbers (a "command") when it has something to say, and the cable hands both games
 * the same "records" - each player's command side by side - in the same order; the master GBA's clock decides that
 * order. Here the master (link id 0: the friend in the lower slot) does the same: every frame it pairs its own oldest
 * command with the oldest the other game sent it (src/platform/tandem_net.c, through the server) into a record -
 * empty if neither has one - uses it at once and sends it on; the other game uses one a frame as they come. So the
 * other game sees the records the same number of frames apart as the master did: the game's own code counts on that
 * (trade.c fills the next part of a party a few frames before the master asks for it). The game's own trade code
 * (the trade screen, the party swap, the save after a trade) then runs unchanged.
 *
 * Asking: A on a friend > Trade or Battle (data/scripts/tandem.inc) asks them; they get "NAME wants to trade" (or "a
 * Single Battle" / "a Double Battle") and say yes or no; then both open the trade screen, or start the Cable Club's
 * link battle. Back to the overworld where they stood when it ends (overworld.c).
 * Brennan 2026-11-27: "Trade or battle anywhere by facing a friend"; "trust your friends" (no checks on Pokemon).
 * T6 (21:20): the asker picks Single or Double; everyone battles at full health and gets their Pokemon back exactly
 * as they were afterwards.
 */
#include "global.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "link.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "reload_save.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "trade.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_controllers.h"
#include "cable_club.h"
#include "load_save.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "constants/battle.h"
#include "constants/cable_club.h"
#include "constants/trainers.h"
#include "tandem_link.h"
#include "tandem_overworld.h"
#include "constants/songs.h"
#include "constants/field_weather.h"
#include "platform/tandem_net.h"
#include "battle_pyramid.h"
#include "battle_util.h"
#include "caps.h"
#include "data.h"
#include "evolution_scene.h"
#include "follower_npc.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon_summary_screen.h"
#include "string_util.h"
#include "strings.h"
#include "trainer_hill.h"
#include "ui_stat_editor.h"
#include "wally_journey.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/hold_effects.h"
#include "constants/script_commands.h"
#include "field_weather.h"
#include "m4a.h"

/* what one friend sends another (TandemNet_SendTo's kind) */
enum
{
    P2P_ASK = 1,  /* "will you trade?" */
    P2P_YES,
    P2P_NO,
    P2P_BUSY,     /* in a menu, a battle, a script: can't be asked now */
    P2P_CANCEL,   /* the asker stopped waiting */
    P2P_GO,       /* the asker got the yes: both open the trade screen */
    P2P_NOGO,     /* a yes that came too late */
    P2P_CANT,           /* a yes, but not enough Pokemon that can battle (a Double Battle needs two) */
    P2P_PARTNER_END,    /* T6b: we aren't partners any more (Team Up again, or another partner) */
    P2P_LINK_OPEN = 16, /* my game opened its link: [my open count] */
    P2P_LINK_CLOSE,     /* ... closed it: [my open count] */
    P2P_LINK_CMD,       /* the other game's command, to the master: [my open count][the one I know of yours][8 numbers] */
    P2P_LINK_REC,       /* a record, from the master: [my open count][yours][the master's 8 numbers][the other's 8] */
};

#define ASK_TIMEOUT (30 * 60)   /* frames waiting for a yes or no */
#define JOIN_TIMEOUT (15 * 60)  /* ... for "join the battle?" (the trainer is waiting) */
#define GO_TIMEOUT  (15 * 60)   /* frames waiting for the asker to start */
#define LINK_QUEUE  1200        /* commands or records waiting (20 s of them) */
#define OPEN_TIMEOUT (20 * 60)  /* frames open without the friend's game opening its own: a communication error */
#define CATCH_UP    4           /* records waiting before the other game skips an empty one a frame (after a delay on
                                 * the way, to catch up again) */

/*
 * Each game opens and closes its link several times in one visit to the trade screen (it closes it after each trade
 * and opens it again). Each opening has a number; a "connection" pairs one opening of mine with one of the friend's.
 * Messages between two players arrive in the order they were sent, so the friend's last records of an opening always
 * come before their next opening's news - and those last records are still used (both games close at the same
 * record, but one PC gets there a little later than the other).
 */
static struct
{
    bool8 active;               /* a link with a friend is set up */
    bool8 myOpen, peerOpen;     /* my game has its link open; the friend's latest opening is still open */
    bool8 established;          /* connected: both open */
    u8 peer, myId;              /* the friend's slot; my link id (0: the lower slot, the master) */
    u16 myGen, peerGen;         /* my opening's number; the friend's latest */
    u16 usedPeerGen;            /* the friend's opening my last connection used (one connection per opening) */
    u16 openWait;               /* frames open and not connected */
    bool8 playersSaved;         /* the friend's details from the first connection are kept (link.c) */
    u8 activity;                /* TANDEM_ACTIVITY_*: a trade, or a battle */
    bool8 overflow;             /* too much waiting: the friend's game or mine is stuck */
    /* the master: its own commands and the other game's, waiting to be paired; the other: records waiting */
    u16 mineHead, mineLen, theirsHead, theirsLen;
    u16 mine[LINK_QUEUE][CMD_LENGTH], theirs[LINK_QUEUE][2 * CMD_LENGTH];
} sLink;

static u8 sAskActivity;                 /* what a friend asking me wants (TANDEM_ACTIVITY_*) */
static u8 sSessionActivity;             /* what the link being set up is for */
static u8 sAnswer[TANDEM_MAX_PLAYERS];   /* each friend's last answer (P2P kind), 0 none */
static s8 sWaitingFor = -1;             /* the friend I asked, while I wait */
static s8 sAskedBy = -1;                /* a friend asking me */
static bool8 sAskShown;                 /* ... and the question is on my screen */
static u16 sAskBusyTimer;

/* T6b: Team Up */
static s8 sPartner = -1;                /* my partner (Team Up on a friend; Brennan 23:3x "Partner up first") */
static bool8 sPartnerEndNotice;         /* my partner stopped: told when I'm free */
static u8 sNoticeName[PLAYER_NAME_LENGTH + 1];
static struct                           /* the trainer of a "join the battle?" (both games) */
{
    u16 trainerId;
    u8 mapGroup, mapNum, localId;
} sJoin;
static bool8 sTeamUpPending;            /* the spotted game: my partner said yes - the next trainer battle is a team-up */
static bool8 sTeamUpBroke;              /* the connection broke during one */
static bool8 sTeamUpResults;            /* a team-up ended: its results wait for the Exp. (Tandem_TeamUpExpStart) */
static u8 sTeamUpOutcome;
static u8 sMyBattler;                   /* my battler in it (0 on the spotted game, 2 on the partner's) */
static u8 sFaced[PARTY_SIZE];           /* each of the trainer's Pokemon: the bits of my party slots that faced it */
static u8 sOppSeen;                     /* the trainer's Pokemon that were out */
static struct { u16 species; u8 level; bool8 fainted; } sOpp[PARTY_SIZE];   /* the trainer's, as the battle ended */
extern const u8 Tandem_EventScript_PartnerEnded[];   /* data/scripts/tandem.inc */
static void StartTeamUpAsPartner(void);
static const u8 sText_ABoosted[] = _(" a boosted");

extern const u8 Tandem_EventScript_Asked[];   /* data/scripts/tandem.inc */

/* (test aid: WJPC_TANDEM_DEBUG=1 - the link's openings, connections and asking, to the game's error output) */
#include <stdio.h>
extern char *getenv(const char *name);   /* (stdlib.h clashes with the game's macros) */
#define LINK_LOG(...) do { static int sDbg = -1; if (sDbg < 0) sDbg = getenv("WJPC_TANDEM_DEBUG") != NULL; \
                           if (sDbg) { fprintf(stderr, "[TandemLink %u %u] ", gMain.vblankCounter2, TandemNet_WallMs() % 100000); \
                                       fprintf(stderr, __VA_ARGS__); fflush(stderr); } } while (0)

static u16 Get16(const u8 *p) { return p[0] | (p[1] << 8); }
static void Put16(u8 *p, u16 v) { p[0] = v; p[1] = v >> 8; }

static void SendSimple(u8 slot, u8 kind)
{
    TandemNet_SendTo(slot, kind, NULL, 0);
}

static void SendGen(u8 kind)
{
    u8 b[2];
    Put16(b, sLink.myGen);
    TandemNet_SendTo(sLink.peer, kind, b, sizeof(b));
}

/* everything that came: trade questions and answers, the friend's link */
static void Pump(void)
{
    unsigned from, kind;
    u8 buf[TANDEM_P2P_MAX];
    int len;

    while ((len = TandemNet_PopP2P(&from, &kind, buf, sizeof(buf))) >= 0)
    {
        if (from >= TANDEM_MAX_PLAYERS)
            continue;
        if (kind != P2P_LINK_CMD && kind != P2P_LINK_REC)
            LINK_LOG("from %u: kind %u (%d bytes)\n", from, kind, len);
        switch (kind)
        {
        case P2P_ASK:
            if (sAskedBy < 0 && sWaitingFor < 0 && !sLink.active
             && !(len >= 1 && buf[0] == TANDEM_ACTIVITY_JOIN && (len < 6 || sPartner != (s8)from)))
            {
                sAskedBy = from;
                sAskActivity = len >= 1 && buf[0] < TANDEM_ACTIVITY_COUNT ? buf[0] : TANDEM_ACTIVITY_TRADE;
                if (sAskActivity == TANDEM_ACTIVITY_JOIN)
                {
                    /* (T6b: [activity][trainer id][map group][map number][the trainer's local id]) */
                    sJoin.trainerId = Get16(buf + 1);
                    sJoin.mapGroup = buf[3];
                    sJoin.mapNum = buf[4];
                    sJoin.localId = buf[5];
                }
                sAskShown = FALSE;
                sAskBusyTimer = 0;
                sAnswer[from] = 0;
            }
            else
                SendSimple(from, P2P_BUSY);
            break;
        case P2P_YES:
            if (sWaitingFor != (s8)from)
            {
                SendSimple(from, P2P_NOGO);   /* (I stopped waiting) */
                break;
            }
            sAnswer[from] = kind;
            break;
        case P2P_CANCEL:
            if (sAskedBy == (s8)from && !sAskShown)
                sAskedBy = -1;
            sAnswer[from] = kind;
            break;
        case P2P_PARTNER_END:
            if (sPartner == (s8)from)
            {
                sPartner = -1;
                sPartnerEndNotice = TRUE;
                TandemOverworld_GetFriendName(from, sNoticeName);
            }
            break;
        case P2P_NO:
        case P2P_CANT:
        case P2P_BUSY:
        case P2P_GO:
        case P2P_NOGO:
            sAnswer[from] = kind;
            break;
        case P2P_LINK_OPEN:
            /* (a connection with it starts when mine is open too: LinkFrame) */
            if (sLink.active && from == sLink.peer && len >= 2)
            {
                sLink.peerOpen = TRUE;
                sLink.peerGen = Get16(buf);
            }
            break;
        case P2P_LINK_CLOSE:
            if (sLink.active && from == sLink.peer && len >= 2 && Get16(buf) == sLink.peerGen)
                sLink.peerOpen = FALSE;
            break;
        case P2P_LINK_CMD:
        case P2P_LINK_REC:
            /* (only from the friend's latest opening, sent connected to my current one; commands to the master,
             * records from it) */
            if (sLink.active && sLink.myOpen && from == sLink.peer
             && Get16(buf) == sLink.peerGen && Get16(buf + 2) == sLink.myGen
             && (kind == P2P_LINK_CMD ? sLink.myId == 0 && len >= 4 + 2 * CMD_LENGTH
                                      : sLink.myId != 0 && len >= 4 + 4 * CMD_LENGTH))
            {
                u16 *dst = sLink.theirs[(sLink.theirsHead + sLink.theirsLen) % LINK_QUEUE];
                u32 i, n = kind == P2P_LINK_CMD ? CMD_LENGTH : 2 * CMD_LENGTH;
                if (sLink.theirsLen >= LINK_QUEUE)
                {
                    sLink.overflow = TRUE;
                    break;
                }
                for (i = 0; i < n; i++)
                    dst[i] = Get16(buf + 4 + 2 * i);
                sLink.theirsLen++;
            }
            break;
        }
    }
}

/* a record (both players' commands, as gRecvCmds holds them) with something in it */
static bool8 RecordHasCommand(const u16 *rec)
{
    u32 i;
    for (i = 0; i < 2 * CMD_LENGTH; i++)
        if (rec[i] != 0)
            return TRUE;
    return FALSE;
}

/* the cable: what LinkMain1 does with the serial hardware, done with the network */
static void LinkFrame(void)
{
    bool8 open = Link_PCIsOpen();
    u32 i, j, status;
    u16 any = 0;
    bool8 nothing = TRUE;
    bool8 gone = !TandemNet_SlotConnected(sLink.peer);

    if (open && !sLink.myOpen)
    {
        sLink.myOpen = TRUE;
        sLink.myGen++;
        sLink.established = FALSE;
        sLink.openWait = 0;
        sLink.mineLen = sLink.theirsLen = 0;
        SendGen(P2P_LINK_OPEN);
        LINK_LOG("my link opened (%u)\n", sLink.myGen);
    }
    else if (!open && sLink.myOpen)
    {
        sLink.myOpen = FALSE;
        sLink.established = FALSE;
        SendGen(P2P_LINK_CLOSE);
        LINK_LOG("my link closed (%u)\n", sLink.myGen);
    }
    if (sLink.myOpen && sLink.peerOpen && !sLink.established && !gone && sLink.peerGen != sLink.usedPeerGen)
    {
        sLink.established = TRUE;
        sLink.usedPeerGen = sLink.peerGen;
        LINK_LOG("connected: mine %u, theirs %u, waiting %u\n", sLink.myGen, sLink.peerGen, sLink.theirsLen);
    }

    for (i = 0; i < MAX_RFU_PLAYERS; i++)
        for (j = 0; j < CMD_LENGTH; j++)
            gRecvCmds[i][j] = 0;
    SIO_MULTI_CNT->id = sLink.myId;
    gShouldAdvanceLinkState = 0;
    if (!sLink.established)
    {
        for (i = 0; i < CMD_LENGTH; i++)
            gSendCmd[i] = 0;
        /* (the friend left, or their game never opened its link: the game's own "communication error") */
        if (open && (gone || ++sLink.openWait > OPEN_TIMEOUT))
        {
            LINK_LOG("no friend's link: error (gone %d)\n", gone);
            gLink.state = LINK_STATE_CONN_ESTABLISHED;
            gLinkStatus = sLink.myId | LINK_STAT_CONN_ESTABLISHED | LINK_STAT_ERROR_HARDWARE;
            return;
        }
        /* (open and waiting for the friend: as a cable still finding the other end) */
        gLinkStatus = sLink.myId;
        return;
    }
    gLink.state = LINK_STATE_CONN_ESTABLISHED;
    gLink.playerCount = 2;
    gLink.localId = sLink.myId;
    gLink.isMaster = sLink.myId == 0 ? LINK_MASTER : LINK_SLAVE;

    /* the command the game made last frame, if it made one (as the cable sends only those) */
    for (i = 0; i < CMD_LENGTH; i++)
        any |= gSendCmd[i];
    if (any && sLink.myId == 0)
    {
        if (sLink.mineLen < LINK_QUEUE)
        {
            u16 *dst = sLink.mine[(sLink.mineHead + sLink.mineLen) % LINK_QUEUE];
            for (i = 0; i < CMD_LENGTH; i++)
                dst[i] = gSendCmd[i];
            sLink.mineLen++;
        }
        else
            sLink.overflow = TRUE;
    }
    else if (any)
    {
        u8 b[4 + 2 * CMD_LENGTH];
        Put16(b, sLink.myGen);
        Put16(b + 2, sLink.peerGen);
        for (i = 0; i < CMD_LENGTH; i++)
            Put16(b + 4 + 2 * i, gSendCmd[i]);
        if (!TandemNet_SendTo(sLink.peer, P2P_LINK_CMD, b, sizeof(b)))
            sLink.overflow = TRUE;
    }
    for (i = 0; i < CMD_LENGTH; i++)
        gSendCmd[i] = 0;

    /* this frame's record: the master makes it (its oldest command and the other's) and sends it on; the other game
     * takes the oldest that came */
    if (sLink.myId == 0)
    {
        u8 b[4 + 4 * CMD_LENGTH];
        for (j = 0; j < CMD_LENGTH; j++)
        {
            gRecvCmds[0][j] = sLink.mineLen > 0 ? sLink.mine[sLink.mineHead][j] : 0;
            gRecvCmds[1][j] = sLink.theirsLen > 0 ? sLink.theirs[sLink.theirsHead][j] : 0;
        }
        if (sLink.mineLen > 0)
        {
            sLink.mineHead = (sLink.mineHead + 1) % LINK_QUEUE;
            sLink.mineLen--;
        }
        if (sLink.theirsLen > 0)
        {
            sLink.theirsHead = (sLink.theirsHead + 1) % LINK_QUEUE;
            sLink.theirsLen--;
        }
        Put16(b, sLink.myGen);
        Put16(b + 2, sLink.peerGen);
        for (j = 0; j < CMD_LENGTH; j++)
        {
            Put16(b + 4 + 2 * j, gRecvCmds[0][j]);
            Put16(b + 4 + 2 * (CMD_LENGTH + j), gRecvCmds[1][j]);
        }
        if (!TandemNet_SendTo(sLink.peer, P2P_LINK_REC, b, sizeof(b)))
            sLink.overflow = TRUE;
        nothing = !RecordHasCommand(&gRecvCmds[0][0]);
    }
    else if (sLink.theirsLen > 0)
    {
        if (sLink.theirsLen > CATCH_UP && !RecordHasCommand(sLink.theirs[sLink.theirsHead]))
        {
            sLink.theirsHead = (sLink.theirsHead + 1) % LINK_QUEUE;
            sLink.theirsLen--;
        }
        for (j = 0; j < CMD_LENGTH; j++)
        {
            gRecvCmds[0][j] = sLink.theirs[sLink.theirsHead][j];
            gRecvCmds[1][j] = sLink.theirs[sLink.theirsHead][CMD_LENGTH + j];
        }
        sLink.theirsHead = (sLink.theirsHead + 1) % LINK_QUEUE;
        sLink.theirsLen--;
        nothing = !RecordHasCommand(&gRecvCmds[0][0]);
    }
    status = sLink.myId | (2 << LINK_STAT_PLAYER_COUNT_SHIFT) | LINK_STAT_CONN_ESTABLISHED;
    if (sLink.myId == 0)
        status |= LINK_STAT_MASTER;
    if (nothing)
        status |= LINK_STAT_RECEIVED_NOTHING;
    /* (the friend left, or something is stuck: the game's own "communication error") */
    {
        /* (test aid: once a second, how much is waiting) */
        static u32 sLast;
        if (gMain.vblankCounter2 - sLast >= 60)
        {
            sLast = gMain.vblankCounter2;
            LINK_LOG("waiting: mine %u theirs %u cb1 %p cb2 %p status %08x\n", sLink.mineLen, sLink.theirsLen,
                     (void *)gMain.callback1, (void *)gMain.callback2, (unsigned)gLinkStatus);
        }
    }
    if (gone || sLink.overflow)
    {
        status |= LINK_STAT_ERROR_HARDWARE;
        LINK_LOG("link error (gone %d, overflow %d)\n", gone, sLink.overflow);
    }
    gLinkStatus = status;
}

/* main.c, every frame (was a stub on PC): the network cable when a friend's link is set up */
bool8 HandleLinkConnection(void)
{
    if (!TandemNet_Active())
        return FALSE;
    Pump();
    if (!sLink.active)
        return FALSE;
    {
        /* (test aid: a frame that took long, and how long the link's part took) */
        static unsigned sLastMs;
        unsigned t0 = TandemNet_WallMs(), t1;
        if (sLastMs != 0 && t0 - sLastMs > 200)
            LINK_LOG("slow frame: %u ms since the last (cb2 %p)\n", t0 - sLastMs, (void *)gMain.callback2);
        LinkFrame();
        LinkMain2(&gMain.heldKeys);
        t1 = TandemNet_WallMs();
        if (t1 - t0 > 50)
            LINK_LOG("slow link part: %u ms\n", t1 - t0);
        sLastMs = t1;
    }
    if ((gLinkStatus & LINK_STAT_RECEIVED_NOTHING) && IsSendingKeysOverCable() == TRUE)
        return TRUE;
    return FALSE;
}

bool8 TandemLink_InSession(void)
{
    return sLink.active;
}

/* ── when the connection breaks ───────────────────────────────────────── */

const u8 gText_TandemLinkLost[] = _("Communication error…\nThe connection to your friend\nwas lost.");
const u8 gText_TandemABtnContinue[] = _("A Button: Continue");

/* trade.c: Pokemon swapped and not saved yet (between the swap at the end of the trade's animation and the save
 * after it) */
static bool8 sTradeUnsaved;

void TandemLink_SetTradeUnsaved(bool8 unsaved)
{
    if (sLink.active)
        sTradeUnsaved = unsaved;
}

/* link.c, the error screen's A: back where the player stood - or, if the connection broke between a trade's swap and
 * its save, from the last save (as the game does after a wireless error: the trade on this side is undone, never half
 * done) */
void TandemLink_LeaveAfterError(void)
{
    bool8 reload = sTradeUnsaved;
    bool8 battle = sLink.activity != TANDEM_ACTIVITY_TRADE;
    bool8 teamUp = sLink.activity == TANDEM_ACTIVITY_JOIN;

    LINK_LOG("leaving after the error (%s)\n", reload ? "back to the last save" : "back to the overworld");
    sTradeUnsaved = FALSE;
    TandemLink_EndSession();
    if (teamUp)
    {
        /* (T6b: a team-up cut short - a real battle, the Pokemon stay as they are; the trainer isn't beaten - the
         * script after it ends there: Tandem_TeamUpAfter) */
        sTeamUpBroke = TRUE;
        sTeamUpResults = FALSE;
        gBattleTypeFlags = 0;
        gMain.inBattle = FALSE;
    }
    else if (battle)
    {
        /* (a battle cut short: the Pokemon and bag as they were before it, as after a finished one) */
        LoadPlayerParty();
        SavePlayerBag();
        gBattleTypeFlags = 0;
        gMain.inBattle = FALSE;
    }
    if (reload)
    {
        gMain.state = 0;
        ReloadSave();
    }
    else
    {
        /* (the error screen cleared callback1: the overworld's own again, or the player can't move) */
        SetMainCallback1(CB1_Overworld);
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
    }
}

bool8 TandemLink_FirstPlayers(void)
{
    if (!sLink.active || sLink.playersSaved)
        return FALSE;
    sLink.playersSaved = TRUE;
    return TRUE;
}

static void BeginSession(u8 peer, u8 myId)
{
    memset(&sLink, 0, sizeof(sLink));
    sTradeUnsaved = FALSE;
    sLink.active = TRUE;
    sLink.peer = peer;
    sLink.myId = myId;
    /* (each player's link id, sent with their details: link battles put each player's battlers by it -
     * battle_controllers.c; the Cable Club sets it from the seat taken) */
    SetLocalLinkPlayerId(sLink.myId);
    LINK_LOG("session with %u, my id %u\n", peer, sLink.myId);
}

void TandemLink_EndSession(void)
{
    LINK_LOG("session over\n");
    if (sLink.myOpen)
        SendGen(P2P_LINK_CLOSE);
    memset(&sLink, 0, sizeof(sLink));
    SIO_MULTI_CNT->id = 0;
    gLink.state = 0;
    gLinkStatus = 0;
}

/* ── asking to trade ──────────────────────────────────────────────────── */

/* enough Pokemon (not Eggs) for that battle: one, or two for a Double Battle (everyone is healed for it) */
static bool8 CanBattle(u8 activity)
{
    u32 i, n = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species != SPECIES_NONE && species != SPECIES_EGG
         && (activity != TANDEM_ACTIVITY_JOIN || GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0))
            n++;
    }
    return n >= (activity == TANDEM_ACTIVITY_DOUBLE ? 2 : 1);
}

/* VAR_0x8005: the battle chosen (TANDEM_ACTIVITY_*); VAR_RESULT: TRUE if my Pokemon are enough for it */
void Tandem_CanBattle(void)
{
    gSpecialVar_Result = CanBattle(gSpecialVar_0x8005);
}

/* (the friend in front of me, or the one who asked me) - VAR_0x8005: what for (TANDEM_ACTIVITY_*) */
void Tandem_Ask(void)
{
    u8 slot = TandemOverworld_TalkSlot();
    u8 activity = gSpecialVar_0x8005 < TANDEM_ACTIVITY_COUNT ? gSpecialVar_0x8005 : TANDEM_ACTIVITY_TRADE;

    sAnswer[slot] = 0;
    sWaitingFor = slot;
    sSessionActivity = activity;
    TandemNet_SendTo(slot, P2P_ASK, &activity, 1);
}

#define tTimer data[0]

/* VAR_RESULT: 1 yes (they start too), 0 no, 2 busy or no answer, 3 I stopped waiting (B), 4 they haven't the
 * Pokemon for it */
static void Task_WaitAnswer(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 slot = TandemOverworld_TalkSlot();
    u16 result = 0xFFFF;

    if (sAnswer[slot] == P2P_YES)
    {
        SendSimple(slot, P2P_GO);
        result = 1;
    }
    else if (sAnswer[slot] == P2P_NO)
        result = 0;
    else if (sAnswer[slot] == P2P_CANT)
        result = 4;
    else if (sAnswer[slot] == P2P_BUSY || !TandemNet_SlotConnected(slot)
          || ++tTimer > (sSessionActivity == TANDEM_ACTIVITY_JOIN ? JOIN_TIMEOUT : ASK_TIMEOUT))
        result = 2;
    else if (JOY_NEW(B_BUTTON))
        result = 3;
    if (result == 0xFFFF)
        return;
    if (result == 2 || result == 3)
        SendSimple(slot, P2P_CANCEL);
    sWaitingFor = -1;
    gSpecialVar_Result = result;
    ScriptContext_Enable();
    DestroyTask(taskId);
}

void Tandem_WaitAnswer(void)
{
    CreateTask(Task_WaitAnswer, 80);
}

/* free in the overworld: no script, no menu, standing still */
bool8 TandemLink_PlayerFree(void)
{
    return !ScriptContext_IsEnabled() && !ArePlayerFieldControlsLocked()
        && gPlayerAvatar.tileTransitionState == T_NOT_MOVING && gMain.callback2 == CB2_Overworld;
}

/* a friend asked: once I'm free in the overworld, the question (Tandem_EventScript_Asked); busy for a few
 * seconds - in a menu, a battle, a cutscene - and they are told so */
void TandemLink_OverworldUpdate(void)
{
    if (sPartner >= 0 && !TandemNet_SlotConnected(sPartner))
        sPartner = -1;   /* (T6b: my partner left the game - partners no more) */
    if (sPartnerEndNotice && sAskedBy < 0 && TandemLink_PlayerFree())
    {
        sPartnerEndNotice = FALSE;
        StringCopy(gStringVar1, sNoticeName);
        ScriptContext_SetupScript(Tandem_EventScript_PartnerEnded);
        return;
    }
    if (sAskedBy < 0 || sAskShown)
        return;
    if (!TandemNet_SlotConnected(sAskedBy))
    {
        sAskedBy = -1;
        return;
    }
    if (TandemLink_PlayerFree())
    {
        sAskShown = TRUE;
        TandemOverworld_SetTalkSlot(sAskedBy);
        ScriptContext_SetupScript(Tandem_EventScript_Asked);
    }
    else if (++sAskBusyTimer > 3 * 60)
    {
        SendSimple(sAskedBy, P2P_BUSY);
        sAskedBy = -1;
    }
}

/* the asker's name (STR_VAR_1) and what they want (VAR_0x8005: TANDEM_ACTIVITY_*) */
void Tandem_BufferAsker(void)
{
    Tandem_BufferFriendName();
    gSpecialVar_0x8005 = sAskActivity;
    if (sAskActivity == TANDEM_ACTIVITY_JOIN && sJoin.trainerId < TRAINERS_COUNT)
    {
        /* (T6b: "NAME was spotted by CLASS TRAINER!") */
        StringCopy(gStringVar2, GetTrainerClassNameFromId(sJoin.trainerId));
        StringAppend(gStringVar2, gText_Space);
        StringAppend(gStringVar2, GetTrainerNameFromId(sJoin.trainerId));
    }
}

/* the question's answer (VAR_RESULT, from the yes/no box) to the asker; VAR_RESULT 2 if they stopped waiting, 3 if
 * it was yes to a battle my Pokemon aren't enough for */
void Tandem_Answer(void)
{
    u8 slot = TandemOverworld_TalkSlot();

    /* (2: they gave up while the question was up) */
    if (sAnswer[slot] == P2P_CANCEL || !TandemNet_SlotConnected(slot))
        gSpecialVar_Result = 2;
    sAnswer[slot] = 0;
    if (gSpecialVar_Result == 1 && !CanBattle(sAskActivity))
    {
        SendSimple(slot, P2P_CANT);
        gSpecialVar_Result = 3;
    }
    else if (gSpecialVar_Result != 2)
        SendSimple(slot, gSpecialVar_Result == 1 ? P2P_YES : P2P_NO);
    if (gSpecialVar_Result == 1)
        sSessionActivity = sAskActivity;
    if (gSpecialVar_Result != 1)   /* (1: yes, as the yes/no box gives it) */
    {
        sAskedBy = -1;
        sAskShown = FALSE;
    }
}

/* VAR_RESULT: TRUE when the asker starts too */
static void Task_WaitGo(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 slot = TandemOverworld_TalkSlot();
    bool8 done = FALSE, go = FALSE;

    if (sAnswer[slot] == P2P_GO)
        done = go = TRUE;
    else if (sAnswer[slot] == P2P_NOGO || sAnswer[slot] == P2P_CANCEL
          || !TandemNet_SlotConnected(slot) || ++tTimer > GO_TIMEOUT)
        done = TRUE;
    if (!done)
        return;
    sAskedBy = -1;
    sAskShown = FALSE;
    gSpecialVar_Result = go;
    ScriptContext_Enable();
    DestroyTask(taskId);
}

void Tandem_WaitGo(void)
{
    CreateTask(Task_WaitGo, 80);
}

#undef tTimer

/* both said yes: the link with this friend, and the game's trade screen (as the Cable Club's seat starts it) */
#define tState data[0]
static void Task_StartTrade(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        FadeScreen(FADE_TO_BLACK, 0);
        tState++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            gSelectedTradeMonPositions[TRADE_PLAYER] = 0;
            gSelectedTradeMonPositions[TRADE_PARTNER] = 0;
            m4aMPlayAllStop();
            SetMainCallback2(CB2_StartCreateTradeMenu);
            DestroyTask(taskId);
        }
        break;
    }
}
#undef tState

/* a battle: as the Cable Club's seat starts one (cable_club.c Task_StartWiredCableClubBattle), the battle opening the
 * link itself (battle_controllers.c HandleLinkBattleSetup) */
#define tState data[0]
static void Task_StartBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        FadeScreen(FADE_TO_BLACK, 0);
        tState++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            gLinkType = LINKTYPE_BATTLE;
            /* (the Cable Club picks the music by the first player's trainer id, which it has from the room's link;
             * not here yet - the same for both) */
            PlayMapChosenOrBattleBGM(MUS_VS_TRAINER);
            gBattleTypeFlags = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER;
            if (sSessionActivity == TANDEM_ACTIVITY_DOUBLE)
                gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
            CleanupOverworldWindowsAndTilemaps();
            TRAINER_BATTLE_PARAM.opponentA = TRAINER_LINK_OPPONENT;
            SetMainCallback2(CB2_InitBattle);
            gMain.savedCallback = CB2_ReturnFromCableClubBattle;
            DestroyTask(taskId);
        }
        break;
    }
}
#undef tState

/* both said yes: the link with this friend, then the trade screen or the battle */
void Tandem_Start(void)
{
    if (sSessionActivity == TANDEM_ACTIVITY_JOIN)
    {
        StartTeamUpAsPartner();   /* (T6b: my partner's trainer, against both of us) */
        return;
    }
    BeginSession(TandemOverworld_TalkSlot(), TandemNet_MySlot() < TandemOverworld_TalkSlot() ? 0 : 1);
    sLink.activity = sSessionActivity;
    SetWirelessCommType0();
    gSavedLinkPlayerCount = 2;
    if (sSessionActivity == TANDEM_ACTIVITY_TRADE)
    {
        CreateTask(Task_StartTrade, 80);
        return;
    }
    /* (Brennan 21:20: "Full, then put back" - the party as it is kept aside (the Cable Club keeps it the same way,
     * load_save.c), everyone healed for the battle; CB2_ReturnFromCableClubBattle puts it back) */
    gSpecialVar_0x8004 = sSessionActivity == TANDEM_ACTIVITY_DOUBLE ? USING_DOUBLE_BATTLE : USING_SINGLE_BATTLE;
    SavePlayerParty();
    HealPlayerParty();
    LoadPlayerBag();
    CreateTask(Task_StartBattle, 80);
}

/* ── T6b: Team Up ─────────────────────────────────────────────────────── */
/*
 * Brennan 2026-11-27: Team Up - "Ask each time" (when a regular trainer spots one of you, the other is asked "Join the
 * battle?"), only a partner ("Partner up first": Team Up on a friend partners you, again stops) and only when "On your
 * screen"; rewards - "Exp. for both, rest for one" (money and the trainer marked beaten only for the one spotted).
 * The battle: the Battle Tower's link multi (two players, battle_main.c) against the route trainer, the spotted
 * player's game the link master (it makes the trainer's party, as any trainer battle, and runs its AI).
 */

/* partners: the friend talked to (VAR_RESULT TRUE if they are mine) */
void Tandem_PartnerState(void)
{
    gSpecialVar_Result = sPartner >= 0 && sPartner == (s8)TandemOverworld_TalkSlot();
}

/* both said yes to "team up": partners - one at a time (an old partner is told) */
void Tandem_SetPartner(void)
{
    u8 slot = TandemOverworld_TalkSlot();

    if (sPartner >= 0 && sPartner != (s8)slot)
        SendSimple(sPartner, P2P_PARTNER_END);
    sPartner = slot;
    LINK_LOG("partner now %u\n", slot);
}

void Tandem_StopPartner(void)
{
    if (sPartner >= 0)
        SendSimple(sPartner, P2P_PARTNER_END);
    sPartner = -1;
}

/* a regular trainer (Brennan 03:05: one-player - Gyms, the rival/neighbor, Team Aqua/Magma, the Elite Four, the
 * Champion, "any turning point battles or major ones"); TRAINER_BATTLE_PARAM as the trainerbattle command set it */
static bool8 InGymMap(void)
{
    static const u16 sGyms[] =
    {
        MAP_RUSTBORO_CITY_GYM, MAP_DEWFORD_TOWN_GYM, MAP_MAUVILLE_CITY_GYM, MAP_LAVARIDGE_TOWN_GYM_1F,
        MAP_LAVARIDGE_TOWN_GYM_B1F, MAP_PETALBURG_CITY_GYM, MAP_FORTREE_CITY_GYM, MAP_MOSSDEEP_CITY_GYM,
        MAP_SOOTOPOLIS_CITY_GYM_1F, MAP_SOOTOPOLIS_CITY_GYM_B1F,
        MAP_PEWTER_CITY_GYM, MAP_CERULEAN_CITY_GYM, MAP_VERMILION_CITY_GYM, MAP_CELADON_CITY_GYM,
        MAP_FUCHSIA_CITY_GYM, MAP_SAFFRON_CITY_GYM, MAP_CINNABAR_ISLAND_GYM, MAP_VIRIDIAN_CITY_GYM,
    };
    u16 here = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sGyms); i++)
        if (sGyms[i] == here)
            return TRUE;
    return FALSE;
}

static bool8 IsTeamUpTrainer(void)
{
    u16 id = TRAINER_BATTLE_PARAM.opponentA;

    if (id == TRAINER_NONE || id >= TRAINERS_COUNT)
        return FALSE;   /* (the Secret Base's, the Frontier's, a link's) */
    if (TRAINER_BATTLE_PARAM.opponentB != 0 || gNoOfApproachingTrainers > 1)
        return FALSE;   /* (two trainers at once) */
    if (TRAINER_BATTLE_PARAM.introTextA == NULL || TRAINER_BATTLE_PARAM.earlyRival)
        return FALSE;   /* (a story battle: no intro line) */
    if (InTrainerHillChallenge() || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE
     || FollowerNPCIsBattlePartner() || InGymMap())
        return FALSE;
    switch (GetTrainerClassFromId(id))
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_PKMN_TRAINER_1:
    case TRAINER_CLASS_PKMN_TRAINER_2:
    case TRAINER_CLASS_RS_PROTAG:
    case TRAINER_CLASS_TEAM_AQUA:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_TEAM_MAGMA:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_INTERVIEWER:
    case TRAINER_CLASS_WINSTRATE:
    case TRAINER_CLASS_SALON_MAIDEN:
    case TRAINER_CLASS_DOME_ACE:
    case TRAINER_CLASS_PALACE_MAVEN:
    case TRAINER_CLASS_ARENA_TYCOON:
    case TRAINER_CLASS_FACTORY_HEAD:
    case TRAINER_CLASS_PIKE_QUEEN:
    case TRAINER_CLASS_PYRAMID_KING:
    case TRAINER_CLASS_LEADER_FRLG:
    case TRAINER_CLASS_ELITE_FOUR_FRLG:
    case TRAINER_CLASS_CHAMPION_FRLG:
    case TRAINER_CLASS_RIVAL_EARLY_FRLG:
    case TRAINER_CLASS_RIVAL_LATE_FRLG:
    case TRAINER_CLASS_BOSS_FRLG:
    case TRAINER_CLASS_TEAM_ROCKET_FRLG:
    case TRAINER_CLASS_PKMN_PROF_FRLG:
    case TRAINER_CLASS_PLAYER_FRLG:
        return FALSE;
    default:
        return TRUE;
    }
}

/* before a trainer battle (data/scripts/trainer_battle.inc): VAR_RESULT TRUE if my partner can be asked to join - on
 * my screen, a regular trainer, nobody asking anyone */
void Tandem_TeamUpCheck(void)
{
    sTeamUpPending = FALSE;
    sTeamUpBroke = FALSE;
    sTeamUpResults = FALSE;
    gSpecialVar_Result = sPartner >= 0 && TandemNet_SlotConnected(sPartner) && !sLink.active
                      && sAskedBy < 0 && sWaitingFor < 0
                      && TandemOverworld_FriendOnScreen(sPartner) && IsTeamUpTrainer();
    if (gSpecialVar_Result)
        LINK_LOG("team-up possible: trainer %u\n", TRAINER_BATTLE_PARAM.opponentA);
}

/* ask my partner (STR_VAR_1: their name); the answer as Tandem_WaitAnswer gives it */
void Tandem_TeamUpAsk(void)
{
    u8 b[6];

    TandemOverworld_SetTalkSlot(sPartner);
    TandemOverworld_GetFriendName(sPartner, gStringVar1);
    sAnswer[sPartner] = 0;
    sWaitingFor = sPartner;
    sSessionActivity = TANDEM_ACTIVITY_JOIN;
    b[0] = TANDEM_ACTIVITY_JOIN;
    Put16(b + 1, TRAINER_BATTLE_PARAM.opponentA);
    b[3] = gSaveBlock1Ptr->location.mapGroup;
    b[4] = gSaveBlock1Ptr->location.mapNum;
    /* (the trainer's local id: trainerbattle_single leaves it to the one talked to or spotting - gSpecialVar_LastTalked) */
    b[5] = TRAINER_BATTLE_PARAM.objEventLocalIdA != LOCALID_NONE ? TRAINER_BATTLE_PARAM.objEventLocalIdA : gSpecialVar_LastTalked;
    TandemNet_SendTo(sPartner, P2P_ASK, b, sizeof(b));
}

/* they said yes (and were sent the go): the coming trainer battle is a team-up (battle_setup.c) */
void Tandem_TeamUpGo(void)
{
    sTeamUpPending = TRUE;
}

bool8 TandemLink_TeamUpPending(void)
{
    return sTeamUpPending;
}

static void ResetTeamUpRecords(void)
{
    memset(sFaced, 0, sizeof(sFaced));
    memset(sOpp, 0, sizeof(sOpp));
    sOppSeen = 0;
    sMyBattler = MAX_BATTLERS_COUNT;
    sTeamUpResults = FALSE;
    sTeamUpBroke = FALSE;
}

static void SetUpTeamUpLink(u8 myId)
{
    BeginSession(sPartner, myId);
    sLink.activity = TANDEM_ACTIVITY_JOIN;
    SetWirelessCommType0();
    gSavedLinkPlayerCount = 2;
    gLinkType = LINKTYPE_BATTLE;
    ResetTeamUpRecords();
}

/* the spotted game (battle_setup.c BattleSetup_StartTrainerBattle): the link with my partner, this game the master */
void TandemLink_BeginTeamUpBattle(void)
{
    sTeamUpPending = FALSE;
    SetUpTeamUpLink(0);
    LINK_LOG("team-up: my trainer %u, partner %d\n", TRAINER_BATTLE_PARAM.opponentA, sPartner);
}

/* the partner's game: the trainer that spotted my partner - its id, and its lines from its own script in the map it
 * stands in (my copy of the game has the same maps; its script run up to its trainerbattle, as trainer_see.c does) */
static void SetJoinTrainer(void)
{
    const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(sJoin.mapGroup, sJoin.mapNum);
    u32 i;

    InitTrainerBattleParameter();
    if (map != NULL && map->events != NULL)
    {
        for (i = 0; i < map->events->objectEventCount; i++)
        {
            const struct ObjectEventTemplate *t = &map->events->objectEvents[i];
            struct ScriptContext ctx;

            if (t->localId != sJoin.localId || t->script == NULL)
                continue;
            if (RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_SAVE | SCREFF_HARDWARE | SCREFF_TRAINERBATTLE, t->script, &ctx)
             && *ctx.scriptPtr == SCR_OP_TRAINERBATTLE)
                memcpy(gTrainerBattleParameter.data, ctx.scriptPtr + TRAINERBATTLE_OPCODE_OFFSET, sizeof(TrainerBattleParameter));
            break;
        }
    }
    TRAINER_BATTLE_PARAM.opponentA = sJoin.trainerId;
    TRAINER_BATTLE_PARAM.opponentB = 0;
    TRAINER_BATTLE_PARAM.objEventLocalIdA = LOCALID_NONE;   /* (not one of my map's people) */
    TRAINER_BATTLE_PARAM.objEventLocalIdB = LOCALID_NONE;
    gNoOfApproachingTrainers = 0;
    LINK_LOG("joining: trainer %u, its lines %s\n", sJoin.trainerId, TRAINER_BATTLE_PARAM.defeatTextA ? "found" : "not found");
}

/* the partner's game after the battle: as a trainer battle ends (battle_setup.c CB2_EndTrainerBattle) - a loss: back to
 * the Pokemon Center; else back where I stood, the script going on (the Exp.: Tandem_TeamUpAfter) */
static void CB2_TeamUpPartnerEnd(void)
{
    if (gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW || gBattleOutcome == B_OUTCOME_FORFEITED)
    {
        SetMainCallback2(CB2_WhiteOut);
    }
    else
    {
        BattleSetup_DowngradeBadPoison();
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
    }
}

static void StartTeamUpAsPartner(void)
{
    sPartner = TandemOverworld_TalkSlot();   /* (the one who asked: my partner) */
    SetUpTeamUpLink(1);
    SetJoinTrainer();
    BattleSetup_StartTandemTeamUpBattle(CB2_TeamUpPartnerEnd);
}

/* every battle frame, on both games: which of my Pokemon faced which of the trainer's (both out and standing) - the
 * game's own rule for who gets Exp. (battle_util.c UpdateSentPokesToOpponentValue) */
void TandemLink_TeamUpBattleFrame(void)
{
    u32 o, b;
    u8 mine;
    bool8 mineUp;

    {
        /* (test aid, WJPC_TANDEM_DEBUG: where the battle is, once a second) */
        static u32 sLast;
        if (gMain.vblankCounter2 - sLast >= 60)
        {
            sLast = gMain.vblankCounter2;
            LINK_LOG("battle: main %p exec %08x intro %u ctrl %p %p %p %p cmd %u %u %u %u\n", (void *)gBattleMainFunc,
                     (unsigned)gBattleControllerExecFlags, gBattleStruct->eventState.battleIntro,
                     (void *)gBattlerControllerFuncs[0], (void *)gBattlerControllerFuncs[1],
                     (void *)gBattlerControllerFuncs[2], (void *)gBattlerControllerFuncs[3],
                     gBattleResources->bufferA[0][0], gBattleResources->bufferA[1][0],
                     gBattleResources->bufferA[2][0], gBattleResources->bufferA[3][0]);
        }
    }
    if (sMyBattler >= MAX_BATTLERS_COUNT)
    {
        for (b = 0; b < gBattlersCount; b++)
            if (gBattlerBattleController[b] == BATTLE_CONTROLLER_PLAYER)
                sMyBattler = b;
        if (sMyBattler >= MAX_BATTLERS_COUNT)
            return;
    }
    mine = gBattlerPartyIndexes[sMyBattler];
    mineUp = mine < PARTY_SIZE && IsValidForBattle(&gParties[B_TRAINER_PLAYER][mine]);
    for (o = B_BATTLER_1; o < gBattlersCount; o += 2)
    {
        u8 theirs = gBattlerPartyIndexes[o];

        if (theirs >= PARTY_SIZE || !IsValidForBattle(&gParties[B_TRAINER_OPPONENT_A][theirs]))
            continue;
        sOppSeen |= 1 << theirs;
        if (mineUp)
            sFaced[theirs] |= 1 << mine;
    }
}

/* battle_main.c CB2_InitEndLinkBattle, both games, the link closed: the results kept for the Exp.; the game's own
 * end-of-battle care for my party (battle_main.c HandleEndTurn_FinishBattle - on the master only, if at all: forms
 * back, stats worked out again); the trainer's Pokemon seen in my Pokedex; the session over */
void TandemLink_TeamUpBattleOver(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];

        sOpp[i].species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        sOpp[i].level = GetMonData(mon, MON_DATA_LEVEL);
        sOpp[i].fainted = sOpp[i].species != SPECIES_NONE && sOpp[i].species != SPECIES_EGG
                       && GetMonData(mon, MON_DATA_HP) == 0;
        if ((sOppSeen & (1 << i)) || sOpp[i].fainted)
            HandleSetPokedexFlagFromMon(mon, FLAG_SET_SEEN);
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (!TryRevertPartyMonFormChange(i))
            CalculateMonStats(&gParties[B_TRAINER_PLAYER][i]);
    sTeamUpOutcome = gBattleOutcome;
    sTeamUpResults = gBattleOutcome == B_OUTCOME_WON;
    LINK_LOG("team-up over: outcome %u, faced %02x %02x %02x %02x %02x %02x\n", gBattleOutcome,
             sFaced[0], sFaced[1], sFaced[2], sFaced[3], sFaced[4], sFaced[5]);
    gBattleTypeFlags &= ~(BATTLE_TYPE_TANDEM_TEAMUP | BATTLE_TYPE_LINK | BATTLE_TYPE_LINK_IN_BATTLE | BATTLE_TYPE_MULTI);
    TandemLink_EndSession();
}

/* after a trainer battle (the spotted game) or after joining one (the partner's): VAR_RESULT 1 a won team-up (its Exp.
 * to give), 2 a team-up cut short (the script ends: the trainer isn't beaten), else 0 */
void Tandem_TeamUpAfter(void)
{
    if (sTeamUpBroke)
    {
        sTeamUpBroke = FALSE;
        gSpecialVar_Result = 2;
    }
    else
    {
        gSpecialVar_Result = sTeamUpResults ? 1 : 0;
    }
}

/* ── T6b: Exp. for both ───────────────────────────────────────────────── */
/*
 * A link battle gives no Exp. (battle_script_commands.c BattleTypeAllowsExp - and that file's EXP is Project ARIA's
 * claim), so after a won team-up each game gives its own Pokemon theirs, back in the overworld: the same sums as the
 * battle's own (Cmd_getexp: participants, Exp. Share, EXP ALL, the level cap, Lucky Egg and the rest) with this game's
 * own rules, the trainer's fainted Pokemon one by one, as in battle; then the battle's own messages, new moves (the
 * four-move question; the summary screen, as the Move Deleter opens it) and evolutions.
 */
extern void ApplyExperienceMultipliers(s32 *expAmount, u8 expGetterMonId, u8 faintedBattler);   /* battle_script_commands.c */

static enum HoldEffect TeamUpHoldEffect(struct Pokemon *mon)
{
    return GetItemHoldEffect(GetMonData(mon, MON_DATA_HELD_ITEM));
}

enum
{
    EXP_STATE_GAINS,      /* "NAME gained N Exp. Points!", one a Pokemon that faced the trainer's */
    EXP_STATE_REST,       /* "The rest of your team ..." */
    EXP_STATE_LEVELS,     /* "NAME grew to Lv. N!" */
    EXP_STATE_MOVES,      /* new moves, level by level */
    EXP_STATE_DONE,
};

static struct
{
    u8 state;
    u8 mon;
    u32 gain[PARTY_SIZE];
    u8 startLevel[PARTY_SIZE];
    u8 ownLine;           /* bits: Pokemon whose gain gets its own line (they faced the trainer's, or hold an Exp. Share) */
    u8 leveled;           /* bits: Pokemon that grew (evolutions after) */
    bool8 boosted[PARTY_SIZE];
    u8 restText;          /* 1: EXP ALL's line, 2: the Exp. Share's */
    u8 learnLevel;
    bool8 firstMove;
    u16 move;
    u8 evoNext;
} sExp;

/* the Exp. one of the trainer's Pokemon gives my party, given at once (the battle does it as each one faints) */
static void GiveExpFor(u32 o)
{
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    u32 i, sentInBits = sFaced[o], viaSentIn = 0, viaExpShare = 0, eligible = 0;
    u32 calculatedExp, expValue, expShareValue;
    bool32 expAll = WallyJourney_IsExpAllMode();
    u8 savedLevel = gBattleMons[B_BATTLER_1].level;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        enum HoldEffect holdEffect;

        if (!IsValidForBattle(&party[i]))
            continue;
        eligible++;
        if (sentInBits & (1 << i))
            viaSentIn++;
        holdEffect = TeamUpHoldEffect(&party[i]);
        if (holdEffect == HOLD_EFFECT_EXP_SHARE || IsGen6ExpShareEnabled())
            viaExpShare++;
    }

    calculatedExp = gSpeciesInfo[sOpp[o].species].expYield * sOpp[o].level;
    if (GetConfig(B_SCALED_EXP) >= GEN_5 && GetConfig(B_SCALED_EXP) != GEN_6)
        calculatedExp /= 5;
    else
        calculatedExp /= 7;
    if (GetConfig(B_TRAINER_EXP_MULTIPLIER) <= GEN_7)
        calculatedExp = (calculatedExp * 150) / 100;

    if (expAll)
    {
        u32 pool = GetConfig(B_SPLIT_EXP) < GEN_6 ? calculatedExp : calculatedExp * viaSentIn + calculatedExp / 2;
        expValue = SAFE_DIV(pool, eligible);
        if (expValue == 0)
            expValue = 1;
        expShareValue = expValue;
    }
    else if (GetConfig(B_SPLIT_EXP) < GEN_6)
    {
        if (viaExpShare)
        {
            expValue = SAFE_DIV(calculatedExp / 2, viaSentIn);
            expShareValue = SAFE_DIV(calculatedExp / 2, viaExpShare);
            if (expShareValue == 0)
                expShareValue = 1;
        }
        else
        {
            expValue = SAFE_DIV(calculatedExp, viaSentIn);
            expShareValue = 0;
        }
        if (expValue == 0)
            expValue = 1;
    }
    else
    {
        expValue = calculatedExp;
        expShareValue = calculatedExp / 2;
        if (expShareValue == 0)
            expShareValue = 1;
    }

    /* (ApplyExperienceMultipliers reads the fainted one's level from its battler) */
    gBattleMons[B_BATTLER_1].level = sOpp[o].level;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &party[i];
        bool32 wasSentOut = (sentInBits & (1 << i)) != 0;
        enum HoldEffect holdEffect;
        u32 level, exp;
        s32 reward;

        if (!IsValidForBattle(mon))
            continue;
        holdEffect = TeamUpHoldEffect(mon);
        if (!expAll && holdEffect != HOLD_EFFECT_EXP_SHARE && !wasSentOut && !IsGen6ExpShareEnabled())
            continue;
        level = GetMonData(mon, MON_DATA_LEVEL);
        if (level >= MAX_LEVEL)
            continue;
        reward = (wasSentOut || expAll) ? GetSoftLevelCapExpValue(level, expValue) : 0;
        if (!expAll && (holdEffect == HOLD_EFFECT_EXP_SHARE || IsGen6ExpShareEnabled())
         && (B_SPLIT_EXP < GEN_6 || reward == 0))
            reward += GetSoftLevelCapExpValue(level, expShareValue);
        ApplyExperienceMultipliers(&reward, i, B_BATTLER_1);
        exp = GetMonData(mon, MON_DATA_EXP);
        if (B_EXP_CAP_TYPE == EXP_CAP_HARD && reward != 0)
        {
            enum GrowthRate growthRate = gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate;
            u32 levelCap = GetCurrentLevelCap();

            if (level >= levelCap)
                reward = 0;
            else if (gExperienceTables[growthRate][levelCap] < exp + reward)
                reward = gExperienceTables[growthRate][levelCap] - exp;
        }
        if (reward <= 0)
            continue;
        if (wasSentOut || (!expAll && holdEffect == HOLD_EFFECT_EXP_SHARE))
            sExp.ownLine |= 1 << i;
        else
            sExp.restText = expAll ? 1 : 2;
        if (IsTradedMon(mon))
            sExp.boosted[i] = TRUE;
        sExp.gain[i] += reward;
        exp += reward;
        SetMonData(mon, MON_DATA_EXP, &exp);
        CalculateMonStats(mon);
        if (GetMonData(mon, MON_DATA_LEVEL) > level)
        {
            sExp.leveled |= 1 << i;
            AdjustFriendship(mon, FRIENDSHIP_EVENT_GROW_LEVEL);
        }
    }
    gBattleMons[B_BATTLER_1].level = savedLevel;
}

/* after a won team-up: my Pokemon's Exp. given; VAR_RESULT TRUE if there is anything to say */
void Tandem_TeamUpExpStart(void)
{
    u32 i;

    memset(&sExp, 0, sizeof(sExp));
    gSpecialVar_Result = FALSE;
    if (!sTeamUpResults)
        return;
    sTeamUpResults = FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
        sExp.startLevel[i] = GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_LEVEL);
    for (i = 0; i < PARTY_SIZE; i++)
        if (sOpp[i].fainted)
            GiveExpFor(i);
    for (i = 0; i < PARTY_SIZE; i++)
        if (sExp.gain[i] != 0)
            gSpecialVar_Result = TRUE;
    LINK_LOG("team-up Exp.: %u %u %u %u %u %u\n", sExp.gain[0], sExp.gain[1], sExp.gain[2], sExp.gain[3],
             sExp.gain[4], sExp.gain[5]);
}

static void BufferMonAndMove(void)
{
    GetMonNickname(&gParties[B_TRAINER_PLAYER][sExp.mon], gStringVar1);
    StringCopy(gStringVar2, GetMoveName(sExp.move));
}

/* the next thing to say - VAR_RESULT: 0 done (evolutions next), 1 "NAME gained N Exp. Points!" (STR_VAR_3: " a
 * boosted" or nothing), 2 / 3 "the rest of your team..." (EXP ALL / Exp. Share), 4 "NAME grew to Lv. N!", 5 the same
 * with Stat Points, 6 "NAME learned MOVE!", 7 NAME wants to learn MOVE (four moves already) */
void Tandem_TeamUpExpNext(void)
{
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];

    for (;;)
    {
        switch (sExp.state)
        {
        case EXP_STATE_GAINS:
            for (; sExp.mon < PARTY_SIZE; sExp.mon++)
            {
                if (!(sExp.ownLine & (1 << sExp.mon)) || sExp.gain[sExp.mon] == 0)
                    continue;
                GetMonNickname(&party[sExp.mon], gStringVar1);
                ConvertIntToDecimalStringN(gStringVar2, sExp.gain[sExp.mon], STR_CONV_MODE_LEFT_ALIGN, 7);
                StringCopy(gStringVar3, sExp.boosted[sExp.mon] ? sText_ABoosted : gText_EmptyString2);
                sExp.mon++;
                gSpecialVar_Result = 1;
                return;
            }
            sExp.state = EXP_STATE_REST;
            break;
        case EXP_STATE_REST:
            sExp.state = EXP_STATE_LEVELS;
            sExp.mon = 0;
            if (sExp.restText != 0)
            {
                gSpecialVar_Result = sExp.restText == 1 ? 2 : 3;
                return;
            }
            break;
        case EXP_STATE_LEVELS:
            for (; sExp.mon < PARTY_SIZE; sExp.mon++)
            {
                u8 level = GetMonData(&party[sExp.mon], MON_DATA_LEVEL);

                if (level <= sExp.startLevel[sExp.mon])
                    continue;
                GetMonNickname(&party[sExp.mon], gStringVar1);
                ConvertIntToDecimalStringN(gStringVar2, level, STR_CONV_MODE_LEFT_ALIGN, 3);
                sExp.learnLevel = sExp.startLevel[sExp.mon] + 1;
                sExp.firstMove = TRUE;
                sExp.state = EXP_STATE_MOVES;
                gSpecialVar_Result = StatEditor_LevelUpGivesPoints(sExp.startLevel[sExp.mon], level) ? 5 : 4;
                return;
            }
            sExp.state = EXP_STATE_DONE;
            break;
        case EXP_STATE_MOVES:
        {
            struct Pokemon *mon = &party[sExp.mon];
            u8 level = GetMonData(mon, MON_DATA_LEVEL);

            while (sExp.learnLevel <= level)
            {
                enum Move move = MonTryLearningNewMoveAtLevel(mon, sExp.firstMove, sExp.learnLevel);

                sExp.firstMove = FALSE;
                while (move == MON_ALREADY_KNOWS_MOVE)
                    move = MonTryLearningNewMoveAtLevel(mon, FALSE, sExp.learnLevel);
                if (move == MOVE_NONE)
                {
                    sExp.learnLevel++;
                    sExp.firstMove = TRUE;
                    continue;
                }
                sExp.move = move == MON_HAS_MAX_MOVES ? gMoveToLearn : move;
                BufferMonAndMove();
                gSpecialVar_Result = move == MON_HAS_MAX_MOVES ? 7 : 6;
                return;
            }
            sExp.mon++;
            sExp.state = EXP_STATE_LEVELS;
            break;
        }
        default:
            gSpecialVar_Result = 0;
            return;
        }
    }
}

/* "forget a move?" yes: the summary screen picks the move (as the Move Deleter opens it, back to the script after) */
void Tandem_TeamUpForgetMove(void)
{
    ShowSelectMovePokemonSummaryScreen(gParties[B_TRAINER_PLAYER], sExp.mon, CB2_ReturnToField, sExp.move);
    gFieldCallback = FieldCB_ContinueScriptHandleMusic;
}

/* ... and what was picked: VAR_RESULT TRUE if a move was forgotten for the new one (STR_VAR_3: the old one) */
void Tandem_TeamUpForgotMove(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sExp.mon];
    u8 slot = GetMoveSlotToReplace();

    BufferMonAndMove();
    gSpecialVar_Result = FALSE;
    if (slot < MAX_MON_MOVES)
    {
        StringCopy(gStringVar3, GetMoveName(GetMonData(mon, MON_DATA_MOVE1 + slot)));
        RemoveMonPPBonus(mon, slot);
        SetMonMoveSlot(mon, sExp.move, slot);
        gSpecialVar_Result = TRUE;
    }
}

/* "did not learn": nothing to do - the next one (Tandem_TeamUpExpNext) */
void Tandem_TeamUpSkipMove(void)
{
    BufferMonAndMove();
}

/* the evolutions after it (battle_main.c TryEvolvePokemon's level-up check): VAR_RESULT TRUE when one starts (the
 * script waits; back in the overworld it goes on) */
void Tandem_TeamUpEvolve(void)
{
    gSpecialVar_Result = FALSE;
    for (; sExp.evoNext < PARTY_SIZE; sExp.evoNext++)
    {
        u32 i = sExp.evoNext;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        bool32 canStopEvo = TRUE;
        enum Species species;

        if (!(sExp.leveled & (1 << i)))
            continue;
        species = GetEvolutionTargetSpecies(mon, EVO_MODE_BATTLE_ONLY, sExp.leveled, NULL, &canStopEvo, CHECK_EVO);
        if (species == SPECIES_NONE)
            continue;
        GetEvolutionTargetSpecies(mon, EVO_MODE_BATTLE_ONLY, sExp.leveled, NULL, &canStopEvo, DO_EVO);
        sExp.evoNext++;
        gCB2_AfterEvolution = CB2_ReturnToField;
        gFieldCallback = FieldCB_ContinueScriptHandleMusic;
        BeginEvolutionScene(mon, species, canStopEvo, i);
        gSpecialVar_Result = TRUE;
        return;
    }
}
