/*
 * Project Tandem T1: the network link (tandem_net.h says what and how to set it up).
 *
 * Plain C, no game headers (Makefile.pc builds it without the game's text preprocessor, as mp3_music.c): the game's
 * own headers and Winsock's clash, and nothing here may touch the game's data anyway.
 *
 * One thread does all the network work (select() over every socket, a tenth of a second at a time); the game thread
 * and the main thread only read a status line under a lock. TCP, each message framed as
 *   u16 length (of what follows), u8 type, payload        (little-endian)
 * Messages:
 *   HELLO   joiner -> host: u16 protocol, build[40], name[16]
 *   WELCOME host -> joiner: u8 your slot (1, 2), u8 players allowed, build[40], host's name[16]
 *   REFUSE  host -> joiner: text[96] (why), then the host closes
 *   PEER    host -> everyone: u8 slot, u8 joined (1) or left (0), name[16]
 *   TEXT    either way: text[96] - the test message (and later, maybe, chat)
 *   PING    either way: u32 number, u32 sender's ms; PONG: the same back
 *   BYE     either way: closing on purpose
 *   STATE   either way: u8 slot, u8 map group, u8 map number, s16 x, s16 y, u8 facing, u8 speed, u16 graphics,
 *           u8 elevation, u8 valid - a player's place (T2); the host passes each friend's on to the others
 * S2, saves on the server (only the server program and a game that joins it; numbers little-endian):
 *   SAVE_STATE  server -> player, when they come in: u8 has, u64 hash, u32 size, s64 saved at (seconds since 1970)
 *   SAVE_GET    player -> server: send me my save
 *   SAVE_BEGIN  either way: u64 hash, u32 size, s64 saved at, u64 base (player -> server: the save it replaces;
 *               server -> player: 0) - then SAVE_PART u32 offset + up to SAVE_PART_LEN bytes, in order, then
 *               SAVE_END u64 hash (a SAVE_END with no SAVE_BEGIN: the server has none)
 *   SAVE_STORED server -> player: u8 stored (0: refused - its save isn't the one the player thought), u64 hash
 * A connection that sends nothing for TIMEOUT_MS (pings go every PING_MS) is closed.
 */
#include "tandem_net.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#ifdef _WIN32
#include <direct.h>   /* (_mkdir: S2's saves folder) */
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET TSocket;
#define TSOCK_BAD INVALID_SOCKET
#define TCloseSocket closesocket
#define TSockWouldBlock() (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINPROGRESS)
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
typedef int TSocket;
#define TSOCK_BAD (-1)
#define TCloseSocket close
#define TSockWouldBlock() (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS)
#endif

#include "tandem_build.h"   /* TANDEM_BUILD: the git commit, made by Makefile.pc */

static void PutU16(uint8_t *p, unsigned v);
static unsigned GetU16(const uint8_t *p);

#define PROTOCOL 8   /* (2: STATE; 3: the following Pokemon in STATE; 4: CARD; 5: the outfit in STATE; 6: invite codes;
                        * 7: P2P - one player to another, for trades and battles; 8: SAVE_* - saves on the server) */
#define BUILD_LEN 40
#define NAME_LEN 16
#define INVITE_LEN 16
#define KEY_LEN 40
#define MAX_ACCOUNTS 64
#define TEXT_LEN 96
#define PING_MS 2000
#define TIMEOUT_MS 15000
#define CONNECT_TIMEOUT_MS 10000
#define RETRY_MS 5000
#define BUF_SIZE 4096

enum { MSG_HELLO = 1, MSG_WELCOME, MSG_REFUSE, MSG_PEER, MSG_TEXT, MSG_PING, MSG_PONG, MSG_BYE, MSG_STATE, MSG_CARD, MSG_P2P,
       MSG_SAVE_STATE, MSG_SAVE_GET, MSG_SAVE_BEGIN, MSG_SAVE_PART, MSG_SAVE_END, MSG_SAVE_STORED };
#define SAVE_PART_LEN 240
#define SAVE_PARTS_PER_TURN 16   /* (4 KB each turn of the loop, a turn every millisecond while a save is moving) */
#define STATE_LEN 24
#define STATE_QUEUE 32
#define STATE_RESEND_MS 1000
#define CARD_RESEND_MS 5000   /* (T4: each player's card, again every few seconds - a friend who joins later gets it too) */
/* host: a game that hosts and plays (player 1); server: the server program (WallysJourneyServer.exe), all friends;
 * join: a game that joins either */
enum { MODE_OFF, MODE_HOST, MODE_JOIN, MODE_SERVER };

typedef struct
{
    TSocket sock;
    bool used, hello, connecting;
    unsigned slot;
    char name[NAME_LEN];
    char key[KEY_LEN];   /* (server: the player's key, once they are in) */
    uint8_t in[BUF_SIZE];
    unsigned inLen;
    uint32_t lastHeard, lastPing, pingNo, connectStart;
    /* S2: a save on its way out (a few parts each turn of the loop) and one coming in */
    uint8_t *outSave;
    unsigned outSize, outAt;
    uint8_t *inSave;
    unsigned inSize, inGot;
    uint64_t inHash, inBase;
    int64_t inTime;
} Peer;

static int sMode = MODE_OFF;
static unsigned sPort = TANDEM_DEFAULT_PORT;
static char sJoinAddr[128];
static char sName[NAME_LEN] = "Player";
/* joining: the invite code (first time) and the key the server gave this PC then (every time after) */
static char sInvite[INVITE_LEN];
static char sKey[KEY_LEN];
static bool sConfigChanged;   /* (the key came: the settings file is written again) */
/* server: its invite code, and the players it knows (key, name), kept in sAccountsPath */
static char sServerInvite[INVITE_LEN];
static struct { char key[KEY_LEN]; char name[NAME_LEN]; } sAccounts[MAX_ACCOUNTS];
static unsigned sAccountCount;
static char sAccountsPath[1100];
static bool sEcho;   /* (server: the log also to its window) */
static char sLogPath[1100];
static FILE *sLog;

static SDL_Thread *sThread;
static SDL_atomic_t sQuit;
static SDL_mutex *sLock;
static char sStatus[128];
static unsigned sStatusGen;

static TSocket sListen = TSOCK_BAD;
static Peer sPeers[TANDEM_MAX_PLAYERS];   /* host: friends in 1..; joiner: the host in 0 */
static char sPeerNames[TANDEM_MAX_PLAYERS][NAME_LEN];   /* everyone connected, by slot ("" free) */
static unsigned sMySlot;

/* T2: places (under sLock): mine, to send; each player's, as received, in order */
static TandemState sMyState, sMySent;
static bool sMyDirty;
static uint32_t sMySentAt;
static TandemState sQueue[TANDEM_MAX_PLAYERS][STATE_QUEUE];
static unsigned sQHead[TANDEM_MAX_PLAYERS], sQLen[TANDEM_MAX_PLAYERS];
static bool sConnected[TANDEM_MAX_PLAYERS];   /* (by slot, not me; read by the game thread) */

/* T4: player cards (under sLock): mine, to send; each friend's latest, by slot (length 0: none yet) */
static uint8_t sMyCard[TANDEM_CARD_MAX];
static unsigned sMyCardLen;
static bool sMyCardDirty;
static uint32_t sMyCardSentAt;
static uint8_t sCards[TANDEM_MAX_PLAYERS][TANDEM_CARD_MAX];
static unsigned sCardLen[TANDEM_MAX_PLAYERS];

/* T5: messages from one player to another (under sLock) - [from][to][kind][bytes]: what came, and what waits to go
 * (the game thread queues, this thread sends) */
#define P2P_QUEUE 4096
typedef struct { uint8_t from, to, kind, len; uint8_t data[TANDEM_P2P_MAX]; } P2PMsg;
static P2PMsg sP2PIn[P2P_QUEUE], sP2POut[P2P_QUEUE];
static unsigned sP2PInHead, sP2PInLen, sP2POutHead, sP2POutLen;

/* S2: saves on the server (under sLock). A joining game: what it knows of the server's save, a fetch, a send waiting
 * to go. The server: the folder it keeps them in. */
static TandemServerSave sSrvSave;
static unsigned sSrvConnection;
static int sGetState;            /* 0 none, 1 asked for (to send), 2 asked, 3 here, -1 failed */
static uint8_t *sGetData;
static unsigned sGetSize;
static uint8_t *sPutData;        /* waiting to go */
static unsigned sPutSize;
static int64_t sPutTime;
static uint64_t sPutBase;
static bool sPutGoing;           /* (sent, waiting for SAVE_STORED) */
static int sPutResult;           /* 0 none, 1 stored, -1 refused or failed */
static uint64_t sPutHash, sPutStoredHash;
static char sSavesDir[1100];

/* ── settings ─────────────────────────────────────────────────────────── */

static void CopyName(char *dst, const char *src)
{
    unsigned n = 0;

    while (*src == ' ' || *src == '\t')
        src++;
    for (; *src != '\0' && *src != '\r' && *src != '\n' && n < NAME_LEN - 1; src++)
        if ((unsigned char)*src >= 0x20 && (unsigned char)*src < 0x7F)
            dst[n++] = *src;
    while (n > 0 && dst[n - 1] == ' ')
        n--;
    dst[n] = '\0';
}

/* an invite code or key: letters and digits only */
static void CopyCode(char *dst, const char *src, unsigned size)
{
    unsigned n = 0;

    for (; *src != '\0' && n < size - 1; src++)
        if ((*src >= '0' && *src <= '9') || (*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z'))
            dst[n++] = *src;
    dst[n] = '\0';
}

static bool SameCode(const char *a, const char *b)
{
    for (; *a != '\0' && *b != '\0'; a++, b++)
        if ((*a | 0x20) != (*b | 0x20))
            return false;
    return *a == *b;
}

static void SetJoin(const char *value)
{
    unsigned n = 0;

    while (*value == ' ')
        value++;
    for (; *value != '\0' && *value != '\r' && *value != '\n' && *value != ' ' && n < sizeof(sJoinAddr) - 1; value++)
        sJoinAddr[n++] = *value;
    sJoinAddr[n] = '\0';
    if (n > 0)
        sMode = MODE_JOIN;
}

bool TandemNet_ConfigLine(const char *line)
{
    unsigned value;

    if (sscanf(line, "tandemHost=%u", &value) == 1)
    {
        if (value > 0 && value < 65536)
        {
            sPort = value;
            sMode = MODE_HOST;
        }
        return true;
    }
    if (strncmp(line, "tandemJoin=", 11) == 0)
    {
        SetJoin(line + 11);
        return true;
    }
    if (strncmp(line, "tandemInvite=", 13) == 0)
    {
        CopyCode(sInvite, line + 13, INVITE_LEN);
        return true;
    }
    if (strncmp(line, "tandemKey=", 10) == 0)
    {
        CopyCode(sKey, line + 10, KEY_LEN);
        return true;
    }
    if (strncmp(line, "tandemName=", 11) == 0)
    {
        char name[NAME_LEN];
        CopyName(name, line + 11);
        if (name[0] != '\0')
            memcpy(sName, name, NAME_LEN);
        return true;
    }
    return false;
}

void TandemNet_WriteConfig(FILE *configFile)
{
    if (sMode == MODE_HOST)
        fprintf(configFile, "tandemHost=%u\n", sPort);
    else if (sMode == MODE_JOIN)
        fprintf(configFile, "tandemJoin=%s\n", sJoinAddr);
    if (strcmp(sName, "Player") != 0)
        fprintf(configFile, "tandemName=%s\n", sName);
    if (sKey[0] != '\0')
        fprintf(configFile, "tandemKey=%s\n", sKey);
    else if (sInvite[0] != '\0')
        fprintf(configFile, "tandemInvite=%s\n", sInvite);
}

bool TandemNet_TakeConfigChanged(void)
{
    bool changed;

    if (sLock == NULL)
        return false;
    SDL_LockMutex(sLock);
    changed = sConfigChanged;
    sConfigChanged = false;
    SDL_UnlockMutex(sLock);
    return changed;
}

/* (tests: the same as environment variables, which win over the file) */
static void ReadEnv(void)
{
    const char *e;

    if ((e = getenv("WJPC_TANDEM_NAME")) != NULL)
    {
        char name[NAME_LEN];
        CopyName(name, e);
        if (name[0] != '\0')
            memcpy(sName, name, NAME_LEN);
    }
    if ((e = getenv("WJPC_TANDEM_HOST")) != NULL && atoi(e) > 0 && atoi(e) < 65536)
    {
        sPort = (unsigned)atoi(e);
        sMode = MODE_HOST;
    }
    if ((e = getenv("WJPC_TANDEM_JOIN")) != NULL)
        SetJoin(e);
    if ((e = getenv("WJPC_TANDEM_INVITE")) != NULL)
        CopyCode(sInvite, e, INVITE_LEN);
    if ((e = getenv("WJPC_TANDEM_KEY")) != NULL)
        CopyCode(sKey, e, KEY_LEN);
}

/* ── log and status ───────────────────────────────────────────────────── */

static SDL_mutex *sLogLock;   /* (S2: the main thread writes to the log too - tandem_save.c) */

static void LogText(const char *text)
{
    time_t now = time(NULL);
    struct tm *t;

    if (sLogLock != NULL)
        SDL_LockMutex(sLogLock);
    t = localtime(&now);
    if (sLog == NULL && sLogPath[0] != '\0')
        sLog = fopen(sLogPath, "a");
    if (sLog != NULL)
    {
        fprintf(sLog, "%02d:%02d:%02d %s\n", t ? t->tm_hour : 0, t ? t->tm_min : 0, t ? t->tm_sec : 0, text);
        fflush(sLog);
    }
    if (sEcho)
    {
        printf("%02d:%02d:%02d %s\n", t ? t->tm_hour : 0, t ? t->tm_min : 0, t ? t->tm_sec : 0, text);
        fflush(stdout);
    }
    else
        fprintf(stderr, "[Tandem] %s\n", text);
    if (sLogLock != NULL)
        SDL_UnlockMutex(sLogLock);
}

static void Log(const char *fmt, ...)
{
    char text[512];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    LogText(text);
}

void TandemNet_Log(const char *fmt, ...)
{
    char text[512];
    va_list ap;

    if (sThread == NULL)
        return;
    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    LogText(text);
}

bool TandemNet_IsJoining(void)
{
    return sThread != NULL && sMode == MODE_JOIN;
}

static void SetStatus(const char *fmt, ...)
{
    char text[sizeof(sStatus)];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    SDL_LockMutex(sLock);
    if (strcmp(text, sStatus) != 0)
    {
        memcpy(sStatus, text, sizeof(sStatus));
        sStatusGen++;
    }
    SDL_UnlockMutex(sLock);
}

const char *TandemNet_StatusText(unsigned *gen)
{
    static char copy[sizeof(sStatus)];

    if (sLock == NULL)
    {
        if (gen != NULL)
            *gen = 0;
        return "";
    }
    SDL_LockMutex(sLock);
    memcpy(copy, sStatus, sizeof(copy));
    if (gen != NULL)
        *gen = sStatusGen;
    SDL_UnlockMutex(sLock);
    return copy;
}

bool TandemNet_Active(void)
{
    return sThread != NULL;
}

/* how many are connected, counting me */
static unsigned CountPlayers(void)
{
    unsigned n = 0;

    for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        if (sPeerNames[i][0] != '\0')
            n++;
    return n;
}

/* the one everyone connects to - a hosting game or the server program - passes everything on */
static bool IsHub(void)
{
    return sMode == MODE_HOST || sMode == MODE_SERVER;
}

/* the first friend's place: after the hosting player, or the first (the server plays no one) */
static unsigned FirstFriendSlot(void)
{
    return sMode == MODE_SERVER ? 0 : 1;
}

static void StatusFromPlayers(void)
{
    char names[3 * NAME_LEN + 8] = "";
    unsigned n = CountPlayers();

    for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        if (sPeerNames[i][0] != '\0' && i != sMySlot)
        {
            if (names[0] != '\0')
                strcat(names, ", ");
            strcat(names, sPeerNames[i]);
        }
    if (sMode == MODE_SERVER)
        SetStatus("server on port %u - %u of %u playing%s%s", sPort, n, TANDEM_MAX_PLAYERS, n ? ": " : "", names);
    else if (sMode == MODE_HOST && n <= 1)
        SetStatus("Tandem: hosting on port %u, waiting for friends", sPort);
    else if (sMode == MODE_HOST)
        SetStatus("Tandem: hosting on port %u - %u of %u (%s)", sPort, n, TANDEM_MAX_PLAYERS, names);
    else
        SetStatus("Tandem: connected - %u of %u (%s)", n, TANDEM_MAX_PLAYERS, names);
}

/* ── places (T2) ──────────────────────────────────────────────────────── */

void TandemNet_SetMyState(const TandemState *state)
{
    if (sLock == NULL)
        return;
    SDL_LockMutex(sLock);
    if (memcmp(state, &sMyState, sizeof(TandemState)) != 0)
    {
        sMyState = *state;
        sMyDirty = true;
    }
    SDL_UnlockMutex(sLock);
}

void TandemNet_SetMyCard(const void *card, unsigned len)
{
    if (sLock == NULL || len > TANDEM_CARD_MAX)
        return;
    SDL_LockMutex(sLock);
    if (len != sMyCardLen || memcmp(card, sMyCard, len) != 0)
    {
        memcpy(sMyCard, card, len);
        sMyCardLen = len;
        sMyCardDirty = true;
    }
    SDL_UnlockMutex(sLock);
}

unsigned TandemNet_GetCard(unsigned slot, void *out, unsigned max)
{
    unsigned len = 0;

    if (sLock == NULL || slot >= TANDEM_MAX_PLAYERS)
        return 0;
    SDL_LockMutex(sLock);
    if (sCardLen[slot] <= max)
    {
        len = sCardLen[slot];
        memcpy(out, sCards[slot], len);
    }
    SDL_UnlockMutex(sLock);
    return len;
}

static bool P2PPush(P2PMsg *q, unsigned *head, unsigned *len, const P2PMsg *m)
{
    if (*len >= P2P_QUEUE)
        return false;
    q[(*head + *len) % P2P_QUEUE] = *m;
    (*len)++;
    return true;
}

static bool P2PPop(P2PMsg *q, unsigned *head, unsigned *len, P2PMsg *m)
{
    if (*len == 0)
        return false;
    *m = q[*head];
    *head = (*head + 1) % P2P_QUEUE;
    (*len)--;
    return true;
}

bool TandemNet_SendTo(unsigned slot, unsigned kind, const void *data, unsigned len)
{
    P2PMsg m;
    bool ok;

    if (sLock == NULL || slot >= TANDEM_MAX_PLAYERS || len > TANDEM_P2P_MAX)
        return false;
    m.from = (uint8_t)sMySlot;
    m.to = (uint8_t)slot;
    m.kind = (uint8_t)kind;
    m.len = (uint8_t)len;
    if (len > 0)
        memcpy(m.data, data, len);
    SDL_LockMutex(sLock);
    ok = P2PPush(sP2POut, &sP2POutHead, &sP2POutLen, &m);
    SDL_UnlockMutex(sLock);
    return ok;
}

int TandemNet_PopP2P(unsigned *fromSlot, unsigned *kind, void *data, unsigned max)
{
    P2PMsg m;
    bool got;

    if (sLock == NULL)
        return -1;
    SDL_LockMutex(sLock);
    got = P2PPop(sP2PIn, &sP2PInHead, &sP2PInLen, &m);
    SDL_UnlockMutex(sLock);
    if (!got)
        return -1;
    *fromSlot = m.from;
    *kind = m.kind;
    if (m.len > max)
        m.len = (uint8_t)max;
    memcpy(data, m.data, m.len);
    return m.len;
}

/* (test aid: milliseconds of the clock on the wall, the same in every game on one PC) */
unsigned TandemNet_WallMs(void)
{
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return (unsigned)((((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime) / 10000);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (unsigned)((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

unsigned TandemNet_MySlot(void)
{
    return sMySlot;
}

bool TandemNet_SlotConnected(unsigned slot)
{
    bool c;

    if (sLock == NULL || slot >= TANDEM_MAX_PLAYERS || slot == sMySlot)
        return false;
    SDL_LockMutex(sLock);
    c = sConnected[slot];
    SDL_UnlockMutex(sLock);
    return c;
}

bool TandemNet_PopState(unsigned slot, TandemState *out)
{
    bool got = false;

    if (sLock == NULL || slot >= TANDEM_MAX_PLAYERS)
        return false;
    SDL_LockMutex(sLock);
    if (sQLen[slot] > 0)
    {
        *out = sQueue[slot][sQHead[slot]];
        sQHead[slot] = (sQHead[slot] + 1) % STATE_QUEUE;
        sQLen[slot]--;
        got = true;
    }
    SDL_UnlockMutex(sLock);
    return got;
}

/* (under sLock) */
static void QueueState(unsigned slot, const TandemState *st)
{
    if (slot >= TANDEM_MAX_PLAYERS)
        return;
    if (sQLen[slot] == STATE_QUEUE)   /* (full: the oldest goes - the game skips ahead anyway) */
    {
        sQHead[slot] = (sQHead[slot] + 1) % STATE_QUEUE;
        sQLen[slot]--;
    }
    sQueue[slot][(sQHead[slot] + sQLen[slot]) % STATE_QUEUE] = *st;
    sQLen[slot]++;
}

static void SetConnected(unsigned slot, bool on)
{
    if (slot >= TANDEM_MAX_PLAYERS)
        return;
    SDL_LockMutex(sLock);
    sConnected[slot] = on;
    if (!on)
        sQLen[slot] = sCardLen[slot] = 0;
    SDL_UnlockMutex(sLock);
}

static void EncodeState(uint8_t *p, unsigned slot, const TandemState *st)
{
    p[0] = (uint8_t)slot;
    p[1] = st->mapGroup;
    p[2] = st->mapNum;
    PutU16(p + 3, (uint16_t)st->x);
    PutU16(p + 5, (uint16_t)st->y);
    p[7] = st->facing;
    p[8] = st->speed;
    PutU16(p + 9, st->gfx);
    p[11] = st->elevation;
    p[12] = st->valid;
    PutU16(p + 13, st->followerGfx);
    PutU16(p + 15, (uint16_t)st->followerX);
    PutU16(p + 17, (uint16_t)st->followerY);
    p[19] = st->followerFacing;
    p[20] = st->outfit;
    p[21] = st->outfitSkin;
    p[22] = st->outfitColour1;
    p[23] = st->outfitColour2;
}

static void DecodeState(const uint8_t *p, TandemState *st)
{
    memset(st, 0, sizeof(*st));
    st->mapGroup = p[1];
    st->mapNum = p[2];
    st->x = (short)GetU16(p + 3);
    st->y = (short)GetU16(p + 5);
    st->facing = p[7];
    st->speed = p[8];
    st->gfx = (unsigned short)GetU16(p + 9);
    st->elevation = p[11];
    st->valid = p[12];
    st->followerGfx = (unsigned short)GetU16(p + 13);
    st->followerX = (short)GetU16(p + 15);
    st->followerY = (short)GetU16(p + 17);
    st->followerFacing = p[19];
    st->outfit = p[20];
    st->outfitSkin = p[21];
    st->outfitColour1 = p[22];
    st->outfitColour2 = p[23];
}

/* ── sockets ──────────────────────────────────────────────────────────── */

static void SetNonBlocking(TSocket s)
{
#ifdef _WIN32
    u_long on = 1;
    ioctlsocket(s, FIONBIO, &on);
#else
    fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
#endif
    {
        int one = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one));
    }
}

static void PutU16(uint8_t *p, unsigned v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void PutU32(uint8_t *p, uint32_t v) { PutU16(p, v & 0xFFFF); PutU16(p + 2, v >> 16); }
static unsigned GetU16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t GetU32(const uint8_t *p) { return GetU16(p) | ((uint32_t)GetU16(p + 2) << 16); }

/* a fixed-length text field: copied and always ended */
static void PutText(uint8_t *p, const char *s, unsigned len)
{
    memset(p, 0, len);
    strncpy((char *)p, s, len - 1);
}

static void GetText(char *dst, const uint8_t *p, unsigned len)
{
    memcpy(dst, p, len);
    dst[len - 1] = '\0';
    for (unsigned i = 0; dst[i] != '\0'; i++)
        if ((unsigned char)dst[i] < 0x20 || (unsigned char)dst[i] >= 0x7F)
            dst[i] = '?';
}

static void ClosePeer(Peer *p, const char *why);
static void EndTransfers(Peer *p);

/* send a whole message (small: the socket's buffer takes it at once; if not, the connection is too far behind) */
static bool Send(Peer *p, unsigned type, const uint8_t *payload, unsigned len)
{
    uint8_t msg[3 + 256];
    unsigned at = 0, total = 3 + len;

    if (!p->used || p->connecting || len > 256)
        return false;
    PutU16(msg, len + 1);
    msg[2] = (uint8_t)type;
    if (len > 0)
        memcpy(msg + 3, payload, len);
    while (at < total)
    {
        int n = send(p->sock, (const char *)msg + at, (int)(total - at), 0);
        if (n <= 0)
        {
            if (n < 0 && TSockWouldBlock())
            {
                SDL_Delay(1);
                continue;
            }
            ClosePeer(p, "the connection broke while sending");
            return false;
        }
        at += (unsigned)n;
    }
    return true;
}

static void SendText(Peer *p, const char *fmt, ...)
{
    uint8_t payload[TEXT_LEN];
    char text[TEXT_LEN];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    PutText(payload, text, TEXT_LEN);
    Send(p, MSG_TEXT, payload, TEXT_LEN);
}

static void SendPeerNews(unsigned slot, bool joined)
{
    uint8_t payload[2 + NAME_LEN];

    payload[0] = (uint8_t)slot;
    payload[1] = joined ? 1 : 0;
    PutText(payload + 2, sPeerNames[slot], NAME_LEN);
    for (unsigned i = FirstFriendSlot(); i < TANDEM_MAX_PLAYERS; i++)
        if (i != slot && sPeers[i].used && sPeers[i].hello)
            Send(&sPeers[i], MSG_PEER, payload, sizeof(payload));
}

static void ClosePeer(Peer *p, const char *why)
{
    if (!p->used)
        return;
    if (p->hello)
        Log("%s left (%s)", p->name[0] ? p->name : "a friend", why);
    else
        Log("connection closed (%s)", why);
    TCloseSocket(p->sock);
    p->used = false;
    EndTransfers(p);
    if (IsHub() && p->hello)
    {
        unsigned slot = p->slot;
        p->hello = false;
        SendPeerNews(slot, false);
        sPeerNames[slot][0] = '\0';
        SetConnected(slot, false);
        StatusFromPlayers();
    }
    else if (sMode == MODE_JOIN)
    {
        for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        {
            sPeerNames[i][0] = '\0';
            SetConnected(i, false);
        }
        SetStatus("Tandem: not connected (%s) - trying again", why);
    }
    p->hello = false;
}

/* ── the server's players (invite codes and keys) ─────────────────────── */

static void LoadAccounts(void)
{
    FILE *f = sAccountsPath[0] ? fopen(sAccountsPath, "r") : NULL;
    char line[256];

    sAccountCount = 0;
    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f) != NULL && sAccountCount < MAX_ACCOUNTS)
    {
        char *space = strchr(line, ' ');
        if (line[0] == '#' || space == NULL)
            continue;
        *space = '\0';
        CopyCode(sAccounts[sAccountCount].key, line, KEY_LEN);
        CopyName(sAccounts[sAccountCount].name, space + 1);
        if (sAccounts[sAccountCount].key[0] != '\0')
            sAccountCount++;
    }
    fclose(f);
}

static int FindAccount(const char *key)
{
    if (key[0] == '\0')
        return -1;
    for (unsigned i = 0; i < sAccountCount; i++)
        if (strcmp(sAccounts[i].key, key) == 0)
            return (int)i;
    return -1;
}

/* a new player let in by the invite code: a key of their own, kept for next time */
static bool NewAccount(const char *name, char *key)
{
    static uint64_t x;
    static const char hex[] = "0123456789abcdef";
    FILE *f;

    if (sAccountCount >= MAX_ACCOUNTS)
        return false;
    x ^= SDL_GetPerformanceCounter() ^ ((uint64_t)time(NULL) << 20) ^ (uintptr_t)&x ^ ((uint64_t)SDL_GetTicks() << 40);
    for (unsigned i = 0; i < 32; i++)
    {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        key[i] = hex[(x >> 29) & 15];
    }
    key[32] = '\0';
    memcpy(sAccounts[sAccountCount].key, key, KEY_LEN);
    memcpy(sAccounts[sAccountCount].name, name, NAME_LEN);
    sAccountCount++;
    f = fopen(sAccountsPath, "a");
    if (f != NULL)
    {
        fprintf(f, "%s %s\n", key, name);
        fclose(f);
    }
    return true;
}

static bool KeyInUse(const char *key)
{
    for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        if (sPeers[i].used && sPeers[i].hello && strcmp(sPeers[i].key, key) == 0)
            return true;
    return false;
}

/* ── S2: saves on the server ─────────────────────────────────────────── */

static void PutU64(uint8_t *p, uint64_t v) { PutU32(p, (uint32_t)v); PutU32(p + 4, (uint32_t)(v >> 32)); }
static uint64_t GetU64(const uint8_t *p) { return GetU32(p) | ((uint64_t)GetU32(p + 4) << 32); }

unsigned long long TandemNet_Hash(const void *data, unsigned size)
{
    const uint8_t *b = data;
    uint64_t h = 0xcbf29ce484222325ull;

    for (unsigned i = 0; i < size; i++)
    {
        h ^= b[i];
        h *= 0x100000001b3ull;
    }
    return h;
}

void TandemNet_GetServerSave(TandemServerSave *out)
{
    if (sLock == NULL)
    {
        memset(out, 0, sizeof(*out));
        return;
    }
    SDL_LockMutex(sLock);
    *out = sSrvSave;
    SDL_UnlockMutex(sLock);
}

bool TandemNet_SaveGet(void)
{
    if (sLock == NULL)
        return false;
    SDL_LockMutex(sLock);
    free(sGetData);
    sGetData = NULL;
    sGetSize = 0;
    sGetState = 1;
    SDL_UnlockMutex(sLock);
    return true;
}

int TandemNet_SaveGetResult(unsigned char **data, unsigned *size)
{
    int r;

    if (sLock == NULL)
        return -1;
    SDL_LockMutex(sLock);
    r = sGetState == 3 ? 1 : sGetState < 0 ? -1 : 0;
    if (r == 1)
    {
        *data = sGetData;
        *size = sGetSize;
        sGetData = NULL;
        sGetState = 0;
    }
    else if (r < 0)
        sGetState = 0;
    SDL_UnlockMutex(sLock);
    return r;
}

bool TandemNet_SavePut(const void *data, unsigned size, long long savedAt, unsigned long long base)
{
    uint8_t *copy;

    if (sLock == NULL || size == 0 || size > TANDEM_SAVE_MAX || (copy = malloc(size)) == NULL)
        return false;
    memcpy(copy, data, size);
    SDL_LockMutex(sLock);
    free(sPutData);
    sPutData = copy;
    sPutSize = size;
    sPutTime = savedAt;
    sPutBase = base;
    sPutHash = TandemNet_Hash(copy, size);
    sPutResult = 0;
    SDL_UnlockMutex(sLock);
    return true;
}

int TandemNet_SavePutResult(unsigned long long *hash)
{
    int r;

    if (sLock == NULL)
        return 0;
    SDL_LockMutex(sLock);
    r = sPutResult;
    if (r != 0)
    {
        *hash = sPutStoredHash;
        sPutResult = 0;
    }
    SDL_UnlockMutex(sLock);
    return r;
}

void TandemNet_SetServerSaves(const char *dir)
{
    snprintf(sSavesDir, sizeof(sSavesDir), "%s", dir);
}

/* a peer's transfers end (it left, or the connection broke); a joining game: what it waited for failed */
static void EndTransfers(Peer *p)
{
    free(p->outSave);
    free(p->inSave);
    p->outSave = p->inSave = NULL;
    if (sMode == MODE_JOIN && sLock != NULL)
    {
        SDL_LockMutex(sLock);
        memset(&sSrvSave, 0, sizeof(sSrvSave));
        if (sGetState > 0)
            sGetState = -1;
        if (sPutGoing || sPutData != NULL)   /* (gone or not yet gone: tandem_save.c decides again when it's back) */
        {
            free(sPutData);
            sPutData = NULL;
            sPutGoing = false;
            sPutResult = -1;
            sPutStoredHash = 0;
        }
        SDL_UnlockMutex(sLock);
    }
}

/* start sending a save to a peer (it is taken: freed when sent) */
static void StartSaveOut(Peer *p, uint8_t *data, unsigned size, uint64_t hash, int64_t savedAt, uint64_t base)
{
    uint8_t b[28];

    free(p->outSave);
    p->outSave = data;
    p->outSize = size;
    p->outAt = 0;
    PutU64(b, hash);
    PutU32(b + 8, size);
    PutU64(b + 12, (uint64_t)savedAt);
    PutU64(b + 20, base);
    Send(p, MSG_SAVE_BEGIN, b, sizeof(b));
}

/* a few more parts of what goes out; the end when all went. TRUE while there is more. */
static bool SendSaveParts(Peer *p)
{
    for (unsigned n = 0; n < SAVE_PARTS_PER_TURN && p->used && p->outSave != NULL; n++)
    {
        uint8_t b[4 + SAVE_PART_LEN];
        unsigned len = p->outSize - p->outAt;

        if (len == 0)
        {
            uint8_t e[8];
            PutU64(e, TandemNet_Hash(p->outSave, p->outSize));
            free(p->outSave);
            p->outSave = NULL;
            Send(p, MSG_SAVE_END, e, sizeof(e));
            return false;
        }
        if (len > SAVE_PART_LEN)
            len = SAVE_PART_LEN;
        PutU32(b, p->outAt);
        memcpy(b + 4, p->outSave + p->outAt, len);
        if (!Send(p, MSG_SAVE_PART, b, 4 + len))
            return false;
        p->outAt += len;
    }
    return p->used && p->outSave != NULL;
}

/* the server's files for a player: KEY.sav, KEY.info ("hash size savedAt name"), and every one it replaced as
 * KEY.YYYYMMDD-HHMMSS.sav (never deleted) */
static void SavePath(char *out, size_t size, const char *key, const char *ending)
{
    snprintf(out, size, "%s/%s%s", sSavesDir[0] ? sSavesDir : ".", key, ending);
}

static bool ReadServerInfo(const char *key, uint64_t *hash, unsigned *size, int64_t *savedAt)
{
    char path[1200], line[256];
    FILE *f;
    bool ok = false;

    SavePath(path, sizeof(path), key, ".info");
    if ((f = fopen(path, "r")) == NULL)
        return false;
    if (fgets(line, sizeof(line), f) != NULL)
    {
        char *end;
        *hash = strtoull(line, &end, 16);
        *size = (unsigned)strtoul(end, &end, 10);
        *savedAt = strtoll(end, &end, 10);
        ok = *size > 0;
    }
    fclose(f);
    return ok;
}

static void SendSaveState(Peer *p)
{
    uint8_t b[21];
    uint64_t hash = 0;
    unsigned size = 0;
    int64_t savedAt = 0;

    memset(b, 0, sizeof(b));
    if (ReadServerInfo(p->key, &hash, &size, &savedAt))
    {
        b[0] = 1;
        PutU64(b + 1, hash);
        PutU32(b + 9, size);
        PutU64(b + 13, (uint64_t)savedAt);
    }
    Send(p, MSG_SAVE_STATE, b, sizeof(b));
}

/* the server: a player asked for their save */
static void ServerSendSave(Peer *p)
{
    char path[1200];
    uint64_t hash = 0;
    unsigned size = 0;
    int64_t savedAt = 0;
    uint8_t *data = NULL;
    FILE *f;

    SavePath(path, sizeof(path), p->key, ".sav");
    if (ReadServerInfo(p->key, &hash, &size, &savedAt) && size <= TANDEM_SAVE_MAX && (f = fopen(path, "rb")) != NULL)
    {
        if ((data = malloc(size)) != NULL && (fread(data, 1, size, f) != size || TandemNet_Hash(data, size) != hash))
        {
            free(data);
            data = NULL;
        }
        fclose(f);
    }
    if (data == NULL)
    {
        uint8_t e[8] = { 0 };
        Log("%s asked for their save - there is none%s", p->name, size ? " (its file doesn't match its info)" : "");
        Send(p, MSG_SAVE_END, e, sizeof(e));
        return;
    }
    Log("sending %s their save (%u bytes)", p->name, size);
    StartSaveOut(p, data, size, hash, savedAt, 0);
}

/* the server: a whole save came; keep it (the one it replaces kept too, by date) */
static void ServerStoreSave(Peer *p)
{
    char path[1200], tmp[1200], old[1200], stamp[32];
    uint64_t curHash = 0;
    unsigned curSize = 0;
    int64_t curTime = 0;
    bool has = ReadServerInfo(p->key, &curHash, &curSize, &curTime), ok = false;
    uint8_t r[9];
    FILE *f;
    time_t now = time(NULL);

    /* (only over the save the player's game knew of: a save sent from another PC in between isn't lost) */
    if (has && p->inBase != curHash)
        Log("%s's save not stored: the server's changed since their game last saw it", p->name);
    else
    {
        if (sSavesDir[0] != '\0')   /* (the folder, the first time) */
#ifdef _WIN32
            _mkdir(sSavesDir);
#else
            mkdir(sSavesDir, 0755);
#endif
        SavePath(tmp, sizeof(tmp), p->key, ".sav.part");
        SavePath(path, sizeof(path), p->key, ".sav");
        if ((f = fopen(tmp, "wb")) != NULL)
        {
            ok = fwrite(p->inSave, 1, p->inSize, f) == p->inSize;
            ok = fclose(f) == 0 && ok;
        }
        if (ok && has)
        {
            struct tm *t = localtime(&now);
            strftime(stamp, sizeof(stamp), ".%Y%m%d-%H%M%S.sav", t);
            SavePath(old, sizeof(old), p->key, stamp);
            ok = rename(path, old) == 0;
        }
        if (ok)
            ok = rename(tmp, path) == 0;
        if (ok)
        {
            SavePath(path, sizeof(path), p->key, ".info");
            if ((f = fopen(path, "w")) != NULL)
            {
                fprintf(f, "%016llx %u %lld %s\n", (unsigned long long)p->inHash, p->inSize, (long long)p->inTime, p->name);
                ok = fclose(f) == 0;
            }
            else
                ok = false;
        }
        Log(ok ? "stored %s's save (%u bytes)%s" : "couldn't store %s's save (%u bytes)%s - is the disk full?", p->name,
            p->inSize, has ? ", the one before kept by date" : "");
    }
    r[0] = ok ? 1 : 0;
    PutU64(r + 1, p->inHash);
    Send(p, MSG_SAVE_STORED, r, sizeof(r));
}

/* SAVE_* messages, either side */
static void HandleSaveMessage(Peer *p, unsigned type, const uint8_t *d, unsigned len)
{
    bool server = sMode == MODE_SERVER, joiner = sMode == MODE_JOIN;

    if (!p->hello || (!server && !joiner) || (server && p->key[0] == '\0'))
        return;
    switch (type)
    {
    case MSG_SAVE_STATE:
        if (joiner && len >= 21)
        {
            SDL_LockMutex(sLock);
            sSrvSave.kind = 2;
            sSrvSave.has = d[0];
            sSrvSave.hash = GetU64(d + 1);
            sSrvSave.size = GetU32(d + 9);
            sSrvSave.savedAt = (long long)GetU64(d + 13);
            sSrvSave.connection = ++sSrvConnection;
            SDL_UnlockMutex(sLock);
            Log(d[0] ? "the server has my save (%u bytes)" : "the server has no save of mine yet", GetU32(d + 9));
        }
        break;
    case MSG_SAVE_GET:
        if (server && p->outSave == NULL)
            ServerSendSave(p);
        break;
    case MSG_SAVE_BEGIN:
        if (len >= 28 && GetU32(d + 8) > 0 && GetU32(d + 8) <= TANDEM_SAVE_MAX)
        {
            free(p->inSave);
            p->inSize = GetU32(d + 8);
            p->inGot = 0;
            p->inHash = GetU64(d);
            p->inTime = (int64_t)GetU64(d + 12);
            p->inBase = GetU64(d + 20);
            p->inSave = malloc(p->inSize);
        }
        break;
    case MSG_SAVE_PART:
        if (p->inSave != NULL && len >= 4 && GetU32(d) == p->inGot && p->inGot + (len - 4) <= p->inSize)
        {
            memcpy(p->inSave + p->inGot, d + 4, len - 4);
            p->inGot += len - 4;
        }
        break;
    case MSG_SAVE_END:
    {
        bool whole = p->inSave != NULL && len >= 8 && p->inGot == p->inSize
                  && TandemNet_Hash(p->inSave, p->inSize) == p->inHash && GetU64(d) == p->inHash;

        if (server)
        {
            if (whole)
                ServerStoreSave(p);
            else
                Log("%s's save came broken - not stored", p->name);
            free(p->inSave);
            p->inSave = NULL;
            break;
        }
        SDL_LockMutex(sLock);
        if (sGetState == 2)
        {
            if (whole)
            {
                sGetData = p->inSave;
                sGetSize = p->inSize;
                p->inSave = NULL;
                sGetState = 3;
            }
            else
                sGetState = -1;
        }
        SDL_UnlockMutex(sLock);
        Log(whole ? "got my save from the server (%u bytes)" : "didn't get my save from the server (none there, or it came broken)",
            p->inSize);
        free(p->inSave);
        p->inSave = NULL;
        break;
    }
    case MSG_SAVE_STORED:
        if (joiner && len >= 9)
        {
            SDL_LockMutex(sLock);
            sPutGoing = false;
            sPutResult = d[0] ? 1 : -1;
            sPutStoredHash = GetU64(d + 1);
            if (d[0])
            {
                sSrvSave.has = 1;
                sSrvSave.hash = sPutStoredHash;
            }
            SDL_UnlockMutex(sLock);
            Log(d[0] ? "the server stored my save" : "the server didn't store my save (its own changed, or its disk)");
        }
        break;
    default:
        break;
    }
}

/* a joining game, each turn of the loop: ask for my save, or start sending it */
static void SaveRequests(Peer *p)
{
    bool get = false;
    uint8_t *put = NULL;
    unsigned putSize = 0;
    int64_t putTime = 0;
    uint64_t putBase = 0, putHash = 0;

    if (sMode != MODE_JOIN || !p->used || !p->hello)
        return;
    SDL_LockMutex(sLock);
    if (sSrvSave.kind == 2)
    {
        if (sGetState == 1)
        {
            sGetState = 2;
            get = true;
        }
        if (sPutData != NULL && !sPutGoing && p->outSave == NULL)
        {
            put = sPutData;
            putSize = sPutSize;
            putTime = sPutTime;
            putBase = sPutBase;
            putHash = sPutHash;
            sPutData = NULL;
            sPutGoing = true;
        }
    }
    else if (sSrvSave.kind == 1 && sGetState == 1)
        sGetState = -1;   /* (a friend's game keeps no saves) */
    SDL_UnlockMutex(sLock);
    if (get)
        Send(p, MSG_SAVE_GET, NULL, 0);
    if (put != NULL)
    {
        Log("sending my save to the server (%u bytes)", putSize);
        StartSaveOut(p, put, putSize, putHash, putTime, putBase);
    }
}

/* ── messages ─────────────────────────────────────────────────────────── */

static void Refuse(Peer *p, const char *fmt, ...)
{
    uint8_t payload[TEXT_LEN];
    char text[TEXT_LEN];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    PutText(payload, text, TEXT_LEN);
    Send(p, MSG_REFUSE, payload, TEXT_LEN);
    Log("refused a friend: %s", text);
    TCloseSocket(p->sock);
    p->used = false;
}

static void HandleMessage(Peer *p, unsigned type, const uint8_t *d, unsigned len)
{
    char build[BUILD_LEN], name[NAME_LEN], text[TEXT_LEN];

    p->lastHeard = SDL_GetTicks();
    if (type >= MSG_SAVE_STATE && type <= MSG_SAVE_STORED)
    {
        HandleSaveMessage(p, type, d, len);
        return;
    }
    switch (type)
    {
    case MSG_HELLO:
    {
        char invite[INVITE_LEN], key[KEY_LEN], newKey[KEY_LEN] = "";

        if (!IsHub() || p->hello || len < 2 + BUILD_LEN + NAME_LEN)
            break;
        GetText(build, d + 2, BUILD_LEN);
        GetText(name, d + 2 + BUILD_LEN, NAME_LEN);
        if (GetU16(d) != PROTOCOL)
        {
            Refuse(p, "different Tandem version (host %u, you %u) - update the game", PROTOCOL, GetU16(d));
            return;
        }
        if (strcmp(build, TANDEM_BUILD) != 0)
        {
            Refuse(p, "different game build: host %s, you %s - both need the same version", TANDEM_BUILD, build);
            return;
        }
        if (name[0] == '\0')
            strcpy(name, "Player");
        /* (the server: a player it knows by their key, or a new one with the invite code; the key goes back to them) */
        invite[0] = key[0] = '\0';
        if (len >= 2 + BUILD_LEN + NAME_LEN + INVITE_LEN + KEY_LEN)
        {
            GetText(text, d + 2 + BUILD_LEN + NAME_LEN, INVITE_LEN);
            CopyCode(invite, text, INVITE_LEN);
            GetText(text, d + 2 + BUILD_LEN + NAME_LEN + INVITE_LEN, KEY_LEN);
            CopyCode(key, text, KEY_LEN);
        }
        if (sMode == MODE_SERVER)
        {
            if (FindAccount(key) < 0)
            {
                if (invite[0] == '\0')
                {
                    Refuse(p, "this server needs an invite code the first time - ask the host for it");
                    return;
                }
                if (!SameCode(invite, sServerInvite))
                {
                    Refuse(p, "wrong invite code - ask the host for it");
                    return;
                }
                if (!NewAccount(name, newKey))
                {
                    Refuse(p, "the server has no room for new players");
                    return;
                }
                memcpy(key, newKey, KEY_LEN);
                Log("%s used the invite code - remembered as a player of this server", name);
            }
            else if (KeyInUse(key))
            {
                Refuse(p, "you're already connected from another PC");
                return;
            }
            memcpy(p->key, key, KEY_LEN);
        }
        p->hello = true;
        memcpy(p->name, name, NAME_LEN);
        memcpy(sPeerNames[p->slot], name, NAME_LEN);
        {
            /* slot, players, build, host's name, the hosting player's slot (0xFF: a server, no player), a new key */
            uint8_t w[2 + BUILD_LEN + NAME_LEN + 1 + KEY_LEN];
            w[0] = (uint8_t)p->slot;
            w[1] = TANDEM_MAX_PLAYERS;
            PutText(w + 2, TANDEM_BUILD, BUILD_LEN);
            PutText(w + 2 + BUILD_LEN, sName, NAME_LEN);
            w[2 + BUILD_LEN + NAME_LEN] = sMode == MODE_SERVER ? 0xFF : 0;
            PutText(w + 3 + BUILD_LEN + NAME_LEN, newKey, KEY_LEN);
            Send(p, MSG_WELCOME, w, sizeof(w));
        }
        /* everyone already here, to the newcomer; the newcomer, to everyone */
        for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
            if (i != p->slot && sPeerNames[i][0] != '\0')
            {
                uint8_t n[2 + NAME_LEN];
                n[0] = (uint8_t)i;
                n[1] = 1;
                PutText(n + 2, sPeerNames[i], NAME_LEN);
                Send(p, MSG_PEER, n, sizeof(n));
            }
        SendPeerNews(p->slot, true);
        SetConnected(p->slot, true);
        sMySentAt = 0;   /* (my place to the newcomer at once) */
        Log("%s joined (player %u of %u, same build %s)", name, p->slot + 1, TANDEM_MAX_PLAYERS, TANDEM_BUILD);
        SendText(p, sMode == MODE_SERVER ? "welcome to %s" : "hello from %s, the host", sName);
        if (sMode == MODE_SERVER)
            SendSaveState(p);   /* (S2: whether it has their save, and which) */
        StatusFromPlayers();
        break;
    }
    case MSG_WELCOME:
        if (sMode != MODE_JOIN || p->hello || len < 2 + BUILD_LEN + NAME_LEN)
            break;
        GetText(build, d + 2, BUILD_LEN);
        GetText(name, d + 2 + BUILD_LEN, NAME_LEN);
        if (strcmp(build, TANDEM_BUILD) != 0)
        {
            Log("the host runs a different build (%s, mine %s) - leaving", build, TANDEM_BUILD);
            ClosePeer(p, "different game build");
            return;
        }
        p->hello = true;
        sMySlot = d[0] < TANDEM_MAX_PLAYERS ? d[0] : 1;
        memcpy(p->name, name, NAME_LEN);
        memcpy(sPeerNames[sMySlot], sName, NAME_LEN);
        sMySentAt = sMyCardSentAt = 0;
        if (len >= 3 + BUILD_LEN + NAME_LEN + KEY_LEN)
        {
            char key[KEY_LEN];
            unsigned hostSlot = d[2 + BUILD_LEN + NAME_LEN];

            /* (a hosting game plays too, as that player; a server plays no one) */
            if (hostSlot < TANDEM_MAX_PLAYERS && hostSlot != sMySlot)
            {
                memcpy(sPeerNames[hostSlot], name, NAME_LEN);
                SetConnected(hostSlot, true);
            }
            GetText(text, d + 3 + BUILD_LEN + NAME_LEN, KEY_LEN);
            CopyCode(key, text, KEY_LEN);
            if (key[0] != '\0')
            {
                /* (let in by the invite code: this PC keeps the key; the code isn't needed again) */
                SDL_LockMutex(sLock);
                memcpy(sKey, key, KEY_LEN);
                sInvite[0] = '\0';
                sConfigChanged = true;
                SDL_UnlockMutex(sLock);
                Log("the server let us in with the invite code and gave this PC its key - no code needed from now on");
            }
            if (hostSlot != 0xFF)
            {
                SDL_LockMutex(sLock);
                sSrvSave.kind = 1;   /* (S2: a friend's game - no saves there) */
                sSrvSave.connection = ++sSrvConnection;
                SDL_UnlockMutex(sLock);
            }
            if (hostSlot == 0xFF)
                Log("joined the server %s as player %u of %u (same build %s)", name, sMySlot + 1, d[1], TANDEM_BUILD);
            else
                Log("joined %s's game as player %u of %u (same build %s)", name, sMySlot + 1, d[1], TANDEM_BUILD);
        }
        SendText(p, "hello from %s", sName);
        StatusFromPlayers();
        break;
    case MSG_REFUSE:
        if (len >= TEXT_LEN)
        {
            GetText(text, d, TEXT_LEN);
            Log("the host refused: %s", text);
            SetStatus("Tandem: refused - %s", text);
        }
        TCloseSocket(p->sock);
        p->used = false;
        /* (a different build or version, an invite code: no retrying, it would be refused again; a full game, or
         * still connected from another PC: try again later) */
        if (len < TEXT_LEN || (strncmp((const char *)d, "the game is full", 16) != 0
                            && strncmp((const char *)d, "you're already connected", 24) != 0))
            sMode = MODE_OFF;
        break;
    case MSG_PEER:
        if (sMode != MODE_JOIN || len < 2 + NAME_LEN || d[0] >= TANDEM_MAX_PLAYERS || d[0] == sMySlot)
            break;
        GetText(name, d + 2, NAME_LEN);
        if (d[1])
        {
            memcpy(sPeerNames[d[0]], name, NAME_LEN);
            SetConnected(d[0], true);
            sMySentAt = 0;
            Log("%s is here too (player %u)", name, d[0] + 1);
        }
        else
        {
            Log("%s left", sPeerNames[d[0]][0] ? sPeerNames[d[0]] : name);
            sPeerNames[d[0]][0] = '\0';
            SetConnected(d[0], false);
        }
        StatusFromPlayers();
        break;
    case MSG_TEXT:
        if (len >= TEXT_LEN)
        {
            GetText(text, d, TEXT_LEN);
            Log("message from %s: \"%s\"", p->name[0] ? p->name : "?", text);
        }
        break;
    case MSG_PING:
        if (len >= 8)
            Send(p, MSG_PONG, d, 8);
        break;
    case MSG_PONG:
        if (len >= 8)
        {
            uint32_t no = GetU32(d), rtt = SDL_GetTicks() - GetU32(d + 4);
            /* (every 15th, about every 30 s, to the log: the link's delay) */
            if (no % 15 == 1)
                Log("ping %s: %u ms there and back", p->name, (unsigned)rtt);
        }
        break;
    case MSG_BYE:
        ClosePeer(p, "they closed the game or left");
        break;
    case MSG_STATE:
        if (len >= STATE_LEN && p->hello)
        {
            TandemState st;
            unsigned slot = IsHub() ? p->slot : d[0];

            if (slot >= TANDEM_MAX_PLAYERS || slot == sMySlot)
                break;
            DecodeState(d, &st);
            SDL_LockMutex(sLock);
            QueueState(slot, &st);
            SDL_UnlockMutex(sLock);
            /* (the host passes it on to the other friends, stamped with whose it is) */
            if (IsHub())
            {
                uint8_t fwd[STATE_LEN];
                memcpy(fwd, d, STATE_LEN);
                fwd[0] = (uint8_t)slot;
                for (unsigned i = FirstFriendSlot(); i < TANDEM_MAX_PLAYERS; i++)
                    if (&sPeers[i] != p && sPeers[i].used && sPeers[i].hello)
                        Send(&sPeers[i], MSG_STATE, fwd, STATE_LEN);
            }
        }
        break;
    case MSG_CARD:
        if (len >= 1 && len - 1 <= TANDEM_CARD_MAX && p->hello)
        {
            unsigned slot = IsHub() ? p->slot : d[0];

            if (slot >= TANDEM_MAX_PLAYERS || slot == sMySlot)
                break;
            SDL_LockMutex(sLock);
            memcpy(sCards[slot], d + 1, len - 1);
            sCardLen[slot] = len - 1;
            SDL_UnlockMutex(sLock);
            if (IsHub())
            {
                uint8_t fwd[1 + TANDEM_CARD_MAX];
                memcpy(fwd, d, len);
                fwd[0] = (uint8_t)slot;
                for (unsigned i = FirstFriendSlot(); i < TANDEM_MAX_PLAYERS; i++)
                    if (&sPeers[i] != p && sPeers[i].used && sPeers[i].hello)
                        Send(&sPeers[i], MSG_CARD, fwd, len);
            }
        }
        break;
    case MSG_P2P:
        if (len >= 3 && len - 3 <= TANDEM_P2P_MAX && p->hello)
        {
            P2PMsg m;

            m.from = IsHub() ? (uint8_t)p->slot : d[0];   /* (a hub knows who it came from) */
            m.to = d[1];
            m.kind = d[2];
            m.len = (uint8_t)(len - 3);
            memcpy(m.data, d + 3, m.len);
            if (m.to >= TANDEM_MAX_PLAYERS || m.from >= TANDEM_MAX_PLAYERS)
                break;
            if (m.to == sMySlot)
            {
                SDL_LockMutex(sLock);
                P2PPush(sP2PIn, &sP2PInHead, &sP2PInLen, &m);
                SDL_UnlockMutex(sLock);
            }
            else if (IsHub() && sPeers[m.to].used && sPeers[m.to].hello)
            {
                uint8_t fwd[3 + TANDEM_P2P_MAX];
                fwd[0] = m.from;
                fwd[1] = m.to;
                fwd[2] = m.kind;
                memcpy(fwd + 3, m.data, m.len);
                Send(&sPeers[m.to], MSG_P2P, fwd, 3 + m.len);
            }
        }
        break;
    default:
        break;
    }
}

/* take every whole message out of what has come in; FALSE if the peer went */
static bool TakeMessages(Peer *p)
{
    while (p->used && p->inLen >= 2)
    {
        unsigned len = GetU16(p->in);
        if (len == 0 || len > 1024)
        {
            ClosePeer(p, "a message that made no sense");
            return false;
        }
        if (p->inLen < 2 + len)
            break;
        HandleMessage(p, p->in[2], p->in + 3, len - 1);
        if (!p->used)
            return false;
        memmove(p->in, p->in + 2 + len, p->inLen - 2 - len);
        p->inLen -= 2 + len;
    }
    return p->used;
}

/* read what has come in, handling each whole message as it comes (S2: a save comes as hundreds of messages - more
 * than the buffer holds at once; before, a full buffer closed the connection) */
static void ReadPeer(Peer *p)
{
    bool ended = false;   /* (the other side closed: what came before still counts - its goodbye) */

    for (;;)
    {
        int n;

        if (p->inLen >= BUF_SIZE)
        {
            ClosePeer(p, "a message too long");
            return;
        }
        n = recv(p->sock, (char *)p->in + p->inLen, (int)(BUF_SIZE - p->inLen), 0);
        if (n == 0)
        {
            ended = true;
            break;
        }
        if (n < 0)
        {
            if (!TSockWouldBlock())
                ClosePeer(p, "the connection broke");
            break;
        }
        p->inLen += (unsigned)n;
        if (!TakeMessages(p))
            return;
    }
    if (ended && p->used)
        ClosePeer(p, "the other side closed the connection");
}

/* ── hosting ──────────────────────────────────────────────────────────── */

static bool OpenListen(void)
{
    struct sockaddr_in a;
    int one = 1;

    sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sListen == TSOCK_BAD)
        return false;
    setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons((uint16_t)sPort);
    if (bind(sListen, (struct sockaddr *)&a, sizeof(a)) != 0 || listen(sListen, 4) != 0)
    {
        TCloseSocket(sListen);
        sListen = TSOCK_BAD;
        return false;
    }
    SetNonBlocking(sListen);
    return true;
}

static void AcceptFriend(void)
{
    struct sockaddr_in a;
    socklen_t al = sizeof(a);
    TSocket s = accept(sListen, (struct sockaddr *)&a, &al);
    Peer *p = NULL;

    if (s == TSOCK_BAD)
        return;
    SetNonBlocking(s);
    for (unsigned i = FirstFriendSlot(); i < TANDEM_MAX_PLAYERS && p == NULL; i++)
        if (!sPeers[i].used)
        {
            p = &sPeers[i];
            p->slot = i;
        }
    if (p == NULL)
    {
        Peer tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.sock = s;
        tmp.used = true;
        Refuse(&tmp, "the game is full (%u players)", TANDEM_MAX_PLAYERS);
        return;
    }
    memset(p->name, 0, NAME_LEN);
    memset(p->key, 0, KEY_LEN);
    p->sock = s;
    p->used = true;
    p->hello = false;
    p->connecting = false;
    p->inLen = 0;
    p->lastHeard = p->lastPing = SDL_GetTicks();
    p->pingNo = 0;
    Log("a friend is connecting (player %u's place)", p->slot + 1);
}

/* ── joining ──────────────────────────────────────────────────────────── */

static void SplitAddress(char *host, size_t hostSize, unsigned *port)
{
    const char *colon = strrchr(sJoinAddr, ':');
    size_t n = colon != NULL ? (size_t)(colon - sJoinAddr) : strlen(sJoinAddr);

    *port = TANDEM_DEFAULT_PORT;
    if (colon != NULL && atoi(colon + 1) > 0 && atoi(colon + 1) < 65536)
        *port = (unsigned)atoi(colon + 1);
    if (n >= hostSize)
        n = hostSize - 1;
    memcpy(host, sJoinAddr, n);
    host[n] = '\0';
}

static void StartJoin(void)
{
    Peer *p = &sPeers[0];
    char host[128], portText[8];
    unsigned port;
    struct addrinfo hints, *res = NULL;
    TSocket s;

    SplitAddress(host, sizeof(host), &port);
    snprintf(portText, sizeof(portText), "%u", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, portText, &hints, &res) != 0 || res == NULL)
    {
        Log("can't find the host's address \"%s\"", host);
        SetStatus("Tandem: can't find %s - trying again", host);
        return;
    }
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == TSOCK_BAD)
    {
        freeaddrinfo(res);
        return;
    }
    SetNonBlocking(s);
    if (connect(s, res->ai_addr, (int)res->ai_addrlen) != 0 && !TSockWouldBlock())
    {
        freeaddrinfo(res);
        TCloseSocket(s);
        SetStatus("Tandem: can't reach %s:%u - trying again", host, port);
        return;
    }
    freeaddrinfo(res);
    memset(p, 0, sizeof(*p));
    p->sock = s;
    p->used = true;
    p->connecting = true;
    p->slot = 0;
    p->connectStart = p->lastHeard = p->lastPing = SDL_GetTicks();
    Log("connecting to %s:%u ...", host, port);
    SetStatus("Tandem: connecting to %s:%u", host, port);
}

/* the connect finished (or failed): say hello */
static void FinishJoin(Peer *p)
{
    int err = 0;
    socklen_t el = sizeof(err);
    uint8_t hello[2 + BUILD_LEN + NAME_LEN + INVITE_LEN + KEY_LEN];

    getsockopt(p->sock, SOL_SOCKET, SO_ERROR, (char *)&err, &el);
    if (err != 0)
    {
        TCloseSocket(p->sock);
        p->used = false;
        Log("couldn't connect (is the host's game running and the port open?)");
        SetStatus("Tandem: couldn't connect to %s - trying again", sJoinAddr);
        return;
    }
    p->connecting = false;
    PutU16(hello, PROTOCOL);
    PutText(hello + 2, TANDEM_BUILD, BUILD_LEN);
    PutText(hello + 2 + BUILD_LEN, sName, NAME_LEN);
    SDL_LockMutex(sLock);
    PutText(hello + 2 + BUILD_LEN + NAME_LEN, sInvite, INVITE_LEN);
    PutText(hello + 2 + BUILD_LEN + NAME_LEN + INVITE_LEN, sKey, KEY_LEN);
    SDL_UnlockMutex(sLock);
    Send(p, MSG_HELLO, hello, sizeof(hello));
}

/* ── the thread ───────────────────────────────────────────────────────── */

static int TandemThread(void *unused)
{
    uint32_t nextTry = 0;

    (void)unused;
    if (sMode == MODE_HOST)
    {
        if (!OpenListen())
        {
            Log("can't host on port %u (is another program using it?)", sPort);
            SetStatus("Tandem: can't host on port %u", sPort);
            return 0;
        }
        memcpy(sPeerNames[0], sName, NAME_LEN);
        sMySlot = 0;
        Log("hosting on port %u as %s (build %s)", sPort, sName, TANDEM_BUILD);
        StatusFromPlayers();
    }
    else if (sMode == MODE_SERVER)
    {
        if (!OpenListen())
        {
            Log("can't start the server on port %u (is another program - or another server - using it?)", sPort);
            SetStatus("can't start on port %u", sPort);
            return 0;
        }
        sMySlot = TANDEM_MAX_PLAYERS;   /* (no player of its own) */
        LoadAccounts();
        Log("server running on port %u (build %s), %u player%s known", sPort, TANDEM_BUILD, sAccountCount,
            sAccountCount == 1 ? "" : "s");
        StatusFromPlayers();
    }
    while (!SDL_AtomicGet(&sQuit))
    {
        fd_set rd, wr;
        /* (a hundredth of a second while anyone is connected: a step is a quarter of a second) */
        struct timeval tv = { 0, CountPlayers() > 1 ? 4000 : 100000 };   /* (4 ms: a trade's link sends every frame) */
        TSocket maxS = 0;
        uint32_t now = SDL_GetTicks();
        bool any = false;

        /* (S2: a save moving, or one asked for: a turn every millisecond until it's done) */
        SaveRequests(&sPeers[0]);
        for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
            if (sPeers[i].used && sPeers[i].outSave != NULL && SendSaveParts(&sPeers[i]))
                tv.tv_usec = 1000;
        if (sMode == MODE_JOIN && sLock != NULL)
        {
            SDL_LockMutex(sLock);
            if (sGetState == 1 || (sPutData != NULL && !sPutGoing))
                tv.tv_usec = 1000;
            SDL_UnlockMutex(sLock);
        }

        if (sMode == MODE_OFF)
            break;
        if (sMode == MODE_JOIN && !sPeers[0].used && (int32_t)(now - nextTry) >= 0)
        {
            StartJoin();
            nextTry = now + RETRY_MS;
        }
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        if (sListen != TSOCK_BAD)
        {
            FD_SET(sListen, &rd);
            maxS = sListen;
            any = true;
        }
        for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
            if (sPeers[i].used)
            {
                if (sPeers[i].connecting)
                    FD_SET(sPeers[i].sock, &wr);
                else
                    FD_SET(sPeers[i].sock, &rd);
                maxS = sPeers[i].sock > maxS ? sPeers[i].sock : maxS;
                any = true;
            }
        if (!any)
        {
            SDL_Delay(100);
            continue;
        }
        if (select((int)maxS + 1, &rd, &wr, NULL, &tv) < 0)
        {
            SDL_Delay(50);
            continue;
        }
        if (sListen != TSOCK_BAD && FD_ISSET(sListen, &rd))
            AcceptFriend();
        now = SDL_GetTicks();
        /* (my place, when it changed, and once a second anyway) */
        {
            TandemState mine;
            bool send;
            SDL_LockMutex(sLock);
            mine = sMyState;
            send = sMyDirty || sMySentAt == 0 || (int32_t)(now - sMySentAt) >= STATE_RESEND_MS;
            sMyDirty = false;
            SDL_UnlockMutex(sLock);
            if (send && CountPlayers() > 1 && sMode != MODE_SERVER)
            {
                uint8_t msg[STATE_LEN];
                EncodeState(msg, sMySlot, &mine);
                for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
                    if (sPeers[i].used && sPeers[i].hello)
                        Send(&sPeers[i], MSG_STATE, msg, STATE_LEN);
                sMySentAt = now ? now : 1;
            }
        }
        /* (my card, when it changed, and every few seconds anyway) */
        {
            uint8_t msg[1 + TANDEM_CARD_MAX];
            unsigned cardLen;
            bool send;
            SDL_LockMutex(sLock);
            cardLen = sMyCardLen;
            memcpy(msg + 1, sMyCard, cardLen);
            send = cardLen > 0 && (sMyCardDirty || sMyCardSentAt == 0 || (int32_t)(now - sMyCardSentAt) >= CARD_RESEND_MS);
            sMyCardDirty = false;
            SDL_UnlockMutex(sLock);
            if (send && CountPlayers() > 1)
            {
                msg[0] = (uint8_t)sMySlot;
                for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
                    if (sPeers[i].used && sPeers[i].hello)
                        Send(&sPeers[i], MSG_CARD, msg, 1 + cardLen);
                sMyCardSentAt = now ? now : 1;
            }
        }
        /* (messages for one player: to the hub, which passes them on - or, from the hub, straight to them) */
        for (;;)
        {
            P2PMsg m;
            bool got;
            Peer *to;
            uint8_t msg[3 + TANDEM_P2P_MAX];

            SDL_LockMutex(sLock);
            got = P2PPop(sP2POut, &sP2POutHead, &sP2POutLen, &m);
            SDL_UnlockMutex(sLock);
            if (!got)
                break;
            to = IsHub() ? &sPeers[m.to] : &sPeers[0];
            if (!to->used || !to->hello)
                continue;
            msg[0] = m.from;
            msg[1] = m.to;
            msg[2] = m.kind;
            memcpy(msg + 3, m.data, m.len);
            Send(to, MSG_P2P, msg, 3 + m.len);
        }
        for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        {
            Peer *p = &sPeers[i];

            if (!p->used)
                continue;
            if (p->connecting)
            {
                if (FD_ISSET(p->sock, &wr))
                    FinishJoin(p);
                else if ((int32_t)(now - p->connectStart) > CONNECT_TIMEOUT_MS)
                {
                    TCloseSocket(p->sock);
                    p->used = false;
                    Log("no answer from %s", sJoinAddr);
                    SetStatus("Tandem: no answer from %s - trying again", sJoinAddr);
                }
                continue;
            }
            if (FD_ISSET(p->sock, &rd))
                ReadPeer(p);
            if (!p->used)
                continue;
            /* (signed: what was just read set lastHeard after `now` was taken) */
            if ((int32_t)(now - p->lastHeard) > TIMEOUT_MS)
            {
                ClosePeer(p, "no word for 15 seconds");
                continue;
            }
            if (p->hello && (int32_t)(now - p->lastPing) >= PING_MS)
            {
                uint8_t ping[8];
                p->lastPing = now;
                PutU32(ping, ++p->pingNo);
                PutU32(ping + 4, now);
                Send(p, MSG_PING, ping, 8);
            }
        }
    }
    for (unsigned i = 0; i < TANDEM_MAX_PLAYERS; i++)
        if (sPeers[i].used)
        {
            Send(&sPeers[i], MSG_BYE, NULL, 0);
            TCloseSocket(sPeers[i].sock);
            sPeers[i].used = false;
        }
    if (sListen != TSOCK_BAD)
    {
        TCloseSocket(sListen);
        sListen = TSOCK_BAD;
    }
    return 0;
}

void TandemNet_Start(const char *logPath)
{
    ReadEnv();
    if (sMode == MODE_OFF || sThread != NULL)
        return;
    snprintf(sLogPath, sizeof(sLogPath), "%s", logPath != NULL ? logPath : "WallysJourney_tandem.log");
#ifdef _WIN32
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            Log("Windows networking didn't start");
            return;
        }
    }
#endif
    sLock = SDL_CreateMutex();
    sLogLock = SDL_CreateMutex();
    SDL_AtomicSet(&sQuit, 0);
    memset(sPeers, 0, sizeof(sPeers));
    memset(sPeerNames, 0, sizeof(sPeerNames));
    Log("---- Project Tandem (build %s) ----", TANDEM_BUILD);
    SetStatus(sMode == MODE_HOST ? "Tandem: starting to host" : "Tandem: starting");
    sThread = SDL_CreateThread(TandemThread, "Tandem", NULL);
}

bool TandemNet_StartServer(unsigned port, const char *invite, const char *serverName, const char *accountsPath,
                           const char *logPath)
{
    sMode = MODE_SERVER;
    sPort = port;
    sEcho = true;
    CopyCode(sServerInvite, invite, INVITE_LEN);
    CopyName(sName, serverName);
    snprintf(sAccountsPath, sizeof(sAccountsPath), "%s", accountsPath);
    TandemNet_Start(logPath);
    return sThread != NULL;
}

void TandemNet_Stop(void)
{
    if (sThread == NULL)
        return;
    SDL_AtomicSet(&sQuit, 1);
    SDL_WaitThread(sThread, NULL);
    sThread = NULL;
    Log("stopped");
    if (sLog != NULL)
    {
        fclose(sLog);
        sLog = NULL;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}
