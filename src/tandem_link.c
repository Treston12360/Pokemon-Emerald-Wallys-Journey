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
 * Asking: A on a friend > Trade (data/scripts/tandem.inc) asks them; they get "NAME wants to trade" and say yes or
 * no; then both open the trade screen. Back to the overworld where they stood when it closes (overworld.c).
 * Brennan 2026-11-27: "Trade or battle anywhere by facing a friend"; "trust your friends" (no checks on Pokemon).
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
#include "tandem_link.h"
#include "tandem_overworld.h"
#include "constants/songs.h"
#include "constants/field_weather.h"
#include "platform/tandem_net.h"

/* what one friend sends another (TandemNet_SendTo's kind) */
enum
{
    P2P_TRADE_ASK = 1,  /* "will you trade?" */
    P2P_TRADE_YES,
    P2P_TRADE_NO,
    P2P_TRADE_BUSY,     /* in a menu, a battle, a script: can't be asked now */
    P2P_TRADE_CANCEL,   /* the asker stopped waiting */
    P2P_TRADE_GO,       /* the asker got the yes: both open the trade screen */
    P2P_TRADE_NOGO,     /* a yes that came too late */
    P2P_LINK_OPEN = 16, /* my game opened its link: [my open count] */
    P2P_LINK_CLOSE,     /* ... closed it: [my open count] */
    P2P_LINK_CMD,       /* the other game's command, to the master: [my open count][the one I know of yours][8 numbers] */
    P2P_LINK_REC,       /* a record, from the master: [my open count][yours][the master's 8 numbers][the other's 8] */
};

#define ASK_TIMEOUT (30 * 60)   /* frames waiting for a yes or no */
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
    bool8 overflow;             /* too much waiting: the friend's game or mine is stuck */
    /* the master: its own commands and the other game's, waiting to be paired; the other: records waiting */
    u16 mineHead, mineLen, theirsHead, theirsLen;
    u16 mine[LINK_QUEUE][CMD_LENGTH], theirs[LINK_QUEUE][2 * CMD_LENGTH];
} sLink;

static u8 sAnswer[TANDEM_MAX_PLAYERS];   /* each friend's last answer (P2P kind), 0 none */
static s8 sWaitingFor = -1;             /* the friend I asked, while I wait */
static s8 sAskedBy = -1;                /* a friend asking me */
static bool8 sAskShown;                 /* ... and the question is on my screen */
static u16 sAskBusyTimer;

extern const u8 Tandem_EventScript_TradeAsked[];   /* data/scripts/tandem.inc */

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
        case P2P_TRADE_ASK:
            if (sAskedBy < 0 && sWaitingFor < 0 && !sLink.active)
            {
                sAskedBy = from;
                sAskShown = FALSE;
                sAskBusyTimer = 0;
                sAnswer[from] = 0;
            }
            else
                SendSimple(from, P2P_TRADE_BUSY);
            break;
        case P2P_TRADE_YES:
            if (sWaitingFor != (s8)from)
            {
                SendSimple(from, P2P_TRADE_NOGO);   /* (I stopped waiting) */
                break;
            }
            sAnswer[from] = kind;
            break;
        case P2P_TRADE_CANCEL:
            if (sAskedBy == (s8)from && !sAskShown)
                sAskedBy = -1;
            sAnswer[from] = kind;
            break;
        case P2P_TRADE_NO:
        case P2P_TRADE_BUSY:
        case P2P_TRADE_GO:
        case P2P_TRADE_NOGO:
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
            LINK_LOG("waiting: mine %u theirs %u\n", sLink.mineLen, sLink.theirsLen);
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
    LinkFrame();
    LinkMain2(&gMain.heldKeys);
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

    LINK_LOG("leaving after the error (%s)\n", reload ? "back to the last save" : "back to the overworld");
    sTradeUnsaved = FALSE;
    TandemLink_EndSession();
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

static void BeginSession(u8 peer)
{
    memset(&sLink, 0, sizeof(sLink));
    sTradeUnsaved = FALSE;
    sLink.active = TRUE;
    sLink.peer = peer;
    sLink.myId = TandemNet_MySlot() < peer ? 0 : 1;
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

/* (the friend in front of me, or the one who asked me) */
void Tandem_AskTrade(void)
{
    u8 slot = TandemOverworld_TalkSlot();

    sAnswer[slot] = 0;
    sWaitingFor = slot;
    SendSimple(slot, P2P_TRADE_ASK);
}

#define tTimer data[0]

/* VAR_RESULT: 1 yes (they open the trade screen too), 0 no, 2 busy or no answer, 3 I stopped waiting (B) */
static void Task_WaitTradeAnswer(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 slot = TandemOverworld_TalkSlot();
    u16 result = 0xFFFF;

    if (sAnswer[slot] == P2P_TRADE_YES)
    {
        SendSimple(slot, P2P_TRADE_GO);
        result = 1;
    }
    else if (sAnswer[slot] == P2P_TRADE_NO)
        result = 0;
    else if (sAnswer[slot] == P2P_TRADE_BUSY || !TandemNet_SlotConnected(slot) || ++tTimer > ASK_TIMEOUT)
        result = 2;
    else if (JOY_NEW(B_BUTTON))
        result = 3;
    if (result == 0xFFFF)
        return;
    if (result >= 2)
        SendSimple(slot, P2P_TRADE_CANCEL);
    sWaitingFor = -1;
    gSpecialVar_Result = result;
    ScriptContext_Enable();
    DestroyTask(taskId);
}

void Tandem_WaitTradeAnswer(void)
{
    CreateTask(Task_WaitTradeAnswer, 80);
}

/* free in the overworld: no script, no menu, standing still */
bool8 TandemLink_PlayerFree(void)
{
    return !ScriptContext_IsEnabled() && !ArePlayerFieldControlsLocked()
        && gPlayerAvatar.tileTransitionState == T_NOT_MOVING && gMain.callback2 == CB2_Overworld;
}

/* a friend asked: once I'm free in the overworld, the question (Tandem_EventScript_TradeAsked); busy for a few
 * seconds - in a menu, a battle, a cutscene - and they are told so */
void TandemLink_OverworldUpdate(void)
{
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
        ScriptContext_SetupScript(Tandem_EventScript_TradeAsked);
    }
    else if (++sAskBusyTimer > 3 * 60)
    {
        SendSimple(sAskedBy, P2P_TRADE_BUSY);
        sAskedBy = -1;
    }
}

void Tandem_BufferAsker(void)
{
    Tandem_BufferFriendName();
}

/* the question's answer (VAR_RESULT, from the yes/no box) to the asker; VAR_RESULT 2 if they stopped waiting */
void Tandem_AnswerTrade(void)
{
    u8 slot = TandemOverworld_TalkSlot();

    /* (2: they gave up while the question was up) */
    if (sAnswer[slot] == P2P_TRADE_CANCEL || !TandemNet_SlotConnected(slot))
        gSpecialVar_Result = 2;
    sAnswer[slot] = 0;
    if (gSpecialVar_Result != 2)
        SendSimple(slot, gSpecialVar_Result == 1 ? P2P_TRADE_YES : P2P_TRADE_NO);
    if (gSpecialVar_Result != 1)   /* (1: yes, as the yes/no box gives it) */
    {
        sAskedBy = -1;
        sAskShown = FALSE;
    }
}

/* VAR_RESULT: TRUE when the asker starts too */
static void Task_WaitTradeGo(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 slot = TandemOverworld_TalkSlot();
    bool8 done = FALSE, go = FALSE;

    if (sAnswer[slot] == P2P_TRADE_GO)
        done = go = TRUE;
    else if (sAnswer[slot] == P2P_TRADE_NOGO || sAnswer[slot] == P2P_TRADE_CANCEL
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

void Tandem_WaitTradeGo(void)
{
    CreateTask(Task_WaitTradeGo, 80);
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

void Tandem_StartTrade(void)
{
    BeginSession(TandemOverworld_TalkSlot());
    SetWirelessCommType0();
    gSavedLinkPlayerCount = 2;
    CreateTask(Task_StartTrade, 80);
}
