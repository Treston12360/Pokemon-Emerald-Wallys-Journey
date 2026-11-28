/*
 * Project Tandem S2: saves on the server - what to do with them (tandem_save.h says what and why; tandem_net.c moves
 * the bytes). Plain C, no game headers (Makefile.pc builds it like tandem_net.c): it reads the save file as bytes.
 *
 * Runs on the main thread only: TandemSave_BootCheck before the game thread starts, TandemSave_Update every frame.
 * TandemSave_Touched may come from the game thread (the PC save block is written there) - an atomic.
 *
 * Safety (Brennan's saves): nothing is ever deleted. A save replaced on this PC is kept beside it, by date; one
 * replaced on the server is kept there, by date. The server takes a save only over the one this game last agreed
 * with it on, so a save made on another PC in between is never overwritten without asking.
 */
#include "tandem_save.h"
#include "tandem_net.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <SDL.h>

#define BOOT_WAIT_MS 8000       /* the server's answer at start */
#define FETCH_WAIT_MS 60000     /* the save itself */
#define SETTLE_MS 3000          /* after the last write of a save, before sending it */
#define QUIT_WAIT_MS 5000       /* closing the game: the last save to the server */

/* the game's save file: flash sectors of 0x1000 bytes (include/save.h struct SaveSector: id at 0xFF4, signature at
 * 0xFF8, counter at 0xFFC), 2 slots of 14, then the PC save block */
#define FLASH_SIZE 0x20000
#define SECTOR 0x1000
#define SECTOR_SIGNATURE 0x8012025
#define NUM_SECTORS 28

static char sPath[1024];
static bool sJoining;
static bool sSyncHas;
static uint64_t sSynced;        /* the save this game and the server last agreed on */
static bool sAllowSend;         /* (false: the server's has changed since - not sent over it) */
static unsigned sHandledConnection;
static bool sPutInFlight;
static uint64_t sPutHash;
static SDL_atomic_t sTouchedAt;
static char sNote[160];
static unsigned sNoteGen;

/* ── files ────────────────────────────────────────────────────────────── */

static uint8_t *ReadWhole(const char *path, unsigned *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *data = NULL;
    long n;

    *size = 0;
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) == 0 && (n = ftell(f)) > 0 && (unsigned long)n <= TANDEM_SAVE_MAX
        && fseek(f, 0, SEEK_SET) == 0 && (data = malloc((size_t)n)) != NULL)
    {
        if (fread(data, 1, (size_t)n, f) == (size_t)n)
            *size = (unsigned)n;
        else
        {
            free(data);
            data = NULL;
        }
    }
    fclose(f);
    return data;
}

static int64_t FileTime(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 ? (int64_t)st.st_mtime : (int64_t)time(NULL);
}

static uint32_t U32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }

/* the newest SaveBlock2 sector (id 0) with the save's signature; -1 when there is no save in it at all (a new PC: the
 * game made an empty file, or one of blank flash) */
static long Block2(const uint8_t *data, unsigned size)
{
    long best = -1;
    uint32_t bestCounter = 0;

    for (unsigned i = 0; i < NUM_SECTORS && (i + 1) * SECTOR <= size; i++)
    {
        const uint8_t *s = data + i * SECTOR;
        if (U32(s + 0xFF8) == SECTOR_SIGNATURE && (s[0xFF4] | (s[0xFF5] << 8)) == 0
            && (best < 0 || U32(s + 0xFFC) >= bestCounter))
        {
            best = (long)i * SECTOR;
            bestCounter = U32(s + 0xFFC);
        }
    }
    return best;
}

static bool RealSave(const uint8_t *data, unsigned size)
{
    return data != NULL && Block2(data, size) >= 0;
}

/* "BOOST, 12:34 played, saved 2026-11-28 01:02" (include/global.h SaveBlock2: name at 0, hours at 0x0E, minutes at
 * 0x10; the name in the game's letters - charmap.txt: A-Z 0xBB.., a-z 0xD5.., 0-9 0xA1.., end 0xFF) */
static void Describe(char *out, size_t size, const uint8_t *data, unsigned dataSize, int64_t savedAt)
{
    long b = Block2(data, dataSize);
    char name[16], when[32];
    unsigned n = 0;
    time_t t = (time_t)savedAt;
    struct tm *tm = localtime(&t);

    if (tm != NULL)
        strftime(when, sizeof(when), "%Y-%m-%d %H:%M", tm);
    else
        snprintf(when, sizeof(when), "?");
    if (b < 0)
    {
        snprintf(out, size, "no save, file from %s", when);
        return;
    }
    for (unsigned i = 0; i < 7; i++)
    {
        uint8_t c = data[b + i];
        if (c == 0xFF)
            break;
        name[n++] = c >= 0xBB && c <= 0xD4 ? (char)('A' + c - 0xBB)
                  : c >= 0xD5 && c <= 0xEE ? (char)('a' + c - 0xD5)
                  : c >= 0xA1 && c <= 0xAA ? (char)('0' + c - 0xA1)
                  : c == 0x00 ? ' ' : '?';
    }
    name[n] = '\0';
    snprintf(out, size, "%s, %u:%02u played, saved %s", n ? name : "?", data[b + 0x0E] | (data[b + 0x0F] << 8),
             data[b + 0x10], when);
}

static void SyncPath(char *out, size_t size)
{
    snprintf(out, size, "%s.tandemsync", sPath);
}

static void ReadSync(void)
{
    char path[1100], line[64];
    FILE *f;

    SyncPath(path, sizeof(path));
    sSyncHas = false;
    if ((f = fopen(path, "r")) == NULL)
        return;
    if (fgets(line, sizeof(line), f) != NULL)
    {
        sSynced = strtoull(line, NULL, 16);
        sSyncHas = sSynced != 0;
    }
    fclose(f);
}

static void WriteSync(uint64_t hash)
{
    char path[1100];
    FILE *f;

    sSynced = hash;
    sSyncHas = true;
    SyncPath(path, sizeof(path));
    if ((f = fopen(path, "w")) != NULL)
    {
        fprintf(f, "%016llx\n", (unsigned long long)hash);
        fclose(f);
    }
}

static void SetNote(const char *text)
{
    if (strcmp(text, sNote) == 0)
        return;
    snprintf(sNote, sizeof(sNote), "%s", text);
    sNoteGen++;
    if (text[0] != '\0')
        TandemNet_Log("saves: %s", text);
}

const char *TandemSave_Note(unsigned *gen)
{
    *gen = sNoteGen;
    return sNote;
}

void TandemSave_Touched(void)
{
    uint32_t now = SDL_GetTicks();
    SDL_AtomicSet(&sTouchedAt, (int)(now ? now : 1));
}

/* send this PC's save; `base`: the server's save it replaces (0: none) */
static void Send(const uint8_t *data, unsigned size, uint64_t base)
{
    if (TandemNet_SavePut(data, size, FileTime(sPath), base))
    {
        sPutInFlight = true;
        sPutHash = TandemNet_Hash(data, size);
        TandemNet_Log("saves: sending this PC's save to the server");
    }
}

/* ── at start ─────────────────────────────────────────────────────────── */

static void Title(struct SDL_Window *window, const char *text)
{
    char title[200];
    snprintf(title, sizeof(title), "Pokemon Wally's Journey - %s", text);
    if (window != NULL)
        SDL_SetWindowTitle(window, title);
}

/* wait for the server's save, keeping the window answering; NULL if it didn't come */
static uint8_t *Fetch(struct SDL_Window *window, unsigned *size)
{
    uint8_t *data = NULL;
    uint32_t start = SDL_GetTicks();
    int r = 0;

    Title(window, "Tandem: getting your save from the server...");
    TandemNet_SaveGet();
    while (r == 0 && SDL_GetTicks() - start < FETCH_WAIT_MS)
    {
        SDL_PumpEvents();
        SDL_Delay(10);
        r = TandemNet_SaveGetResult(&data, size);
    }
    return r == 1 ? data : NULL;
}

/* take the server's save: this PC's kept beside it, by date */
static bool Take(const uint8_t *data, unsigned size, unsigned localSize, void (*closeSave)(void))
{
    char backup[1200], stamp[40];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    FILE *f;
    bool ok = false;

    closeSave();
    strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", tm);
    snprintf(backup, sizeof(backup), "%s.before-server-%s", sPath, stamp);
    if (localSize > 0 && rename(sPath, backup) != 0)
    {
        TandemNet_Log("saves: couldn't keep this PC's save aside (%s) - not taking the server's", backup);
        return false;
    }
    if (localSize == 0)
        remove(sPath);   /* (an empty file the game made) */
    if ((f = fopen(sPath, "wb")) != NULL)
    {
        ok = fwrite(data, 1, size, f) == size;
        ok = fclose(f) == 0 && ok;
    }
    if (!ok)
    {
        TandemNet_Log("saves: couldn't write the server's save - this PC's put back");
        remove(sPath);
        if (localSize > 0)
            rename(backup, sPath);
        return false;
    }
    if (localSize > 0)
        TandemNet_Log("saves: took the server's save; this PC's is kept as %s", backup);
    else
        TandemNet_Log("saves: took the server's save (this PC had none)");
    return true;
}

/* both changed since they last agreed: which one to play? 1 this PC's, 2 the server's, 0 neither answered */
static int Ask(struct SDL_Window *window, const char *mine, const char *theirs)
{
    const char *test = getenv("WJPC_TEST_SAVE_CHOICE");
    char text[700];
    SDL_MessageBoxButtonData buttons[2] = {
        { 0, 1, "This PC's save" },
        { 0, 2, "The server's save" },
    };
    SDL_MessageBoxData box;
    int chosen = 0;

    if (test != NULL)
        return strcmp(test, "pc") == 0 ? 1 : strcmp(test, "server") == 0 ? 2 : 0;
    snprintf(text, sizeof(text),
             "Your save on this PC and your save on the server have both been played since they were last the same "
             "(each was played on a different PC, or without the server).\n\n"
             "This PC:    %s\n"
             "The server: %s\n\n"
             "Which one do you want to play? The other one is kept as a backup, not deleted.", mine, theirs);
    memset(&box, 0, sizeof(box));
    box.flags = SDL_MESSAGEBOX_INFORMATION | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
    box.window = window;
    box.title = "Wally's Journey - which save?";
    box.message = text;
    box.numbuttons = 2;
    box.buttons = buttons;
    if (SDL_ShowMessageBox(&box, &chosen) != 0)
        return 0;
    return chosen;
}

bool TandemSave_BootCheck(const char *savePath, struct SDL_Window *window, void (*closeSave)(void))
{
    TandemServerSave srv;
    uint32_t start = SDL_GetTicks();
    uint8_t *local, *theirs = NULL;
    unsigned localSize, theirSize = 0;
    uint64_t localHash;
    bool localReal, localChanged, serverChanged, changed = false;

    snprintf(sPath, sizeof(sPath), "%s", savePath);
    sJoining = TandemNet_IsJoining();
    if (!sJoining)
        return false;
    ReadSync();
    Title(window, "Tandem: checking your save on the server...");
    do
    {
        SDL_PumpEvents();
        SDL_Delay(10);
        TandemNet_GetServerSave(&srv);
    } while (srv.kind == 0 && SDL_GetTicks() - start < BOOT_WAIT_MS);
    if (srv.kind != 2)
    {
        if (srv.kind == 0)
            TandemNet_Log("saves: no answer from the server at start - this PC's save is played; checked again when "
                          "it connects");
        return false;   /* (a friend's game, or no answer: TandemSave_Update takes it from here) */
    }
    sHandledConnection = srv.connection;
    local = ReadWhole(sPath, &localSize);
    localReal = RealSave(local, localSize);
    localHash = localReal ? TandemNet_Hash(local, localSize) : 0;
    localChanged = localReal && (!sSyncHas || localHash != sSynced);
    serverChanged = srv.has && (!sSyncHas || srv.hash != sSynced);
    TandemNet_Log("saves: at start - this PC %s, the server %s, last agreed %s", localReal ? "has a save" : "has none",
                  srv.has ? "has one" : "has none", sSyncHas ? "known" : "never");

    if (localReal && srv.has && localHash == srv.hash)
    {
        WriteSync(localHash);
        sAllowSend = true;
        TandemNet_Log("saves: the same on both");
    }
    else if (!srv.has)
    {
        sAllowSend = true;
        if (localReal)
            Send(local, localSize, 0);
    }
    else if (!localReal || !localChanged)
    {
        /* (only the server's changed - played on another PC - or this PC has none: take it) */
        if ((theirs = Fetch(window, &theirSize)) != NULL && Take(theirs, theirSize, localSize, closeSave))
        {
            WriteSync(srv.hash);
            sAllowSend = true;
        }
        else
            SetNote("Tandem: couldn't get your save from the server - this PC's is played and not sent");
        changed = true;
    }
    else if (!serverChanged)
    {
        sAllowSend = true;
        Send(local, localSize, sSynced);
    }
    else if ((theirs = Fetch(window, &theirSize)) == NULL)
        SetNote("Tandem: both saves changed and the server's didn't come - this PC's is played and not sent; "
                "restart the game to choose");
    else
    {
        char mine[160], their[160];
        int choice;

        Describe(mine, sizeof(mine), local, localSize, FileTime(sPath));
        Describe(their, sizeof(their), theirs, theirSize, srv.savedAt);
        TandemNet_Log("saves: both changed - this PC: %s; the server: %s", mine, their);
        choice = Ask(window, mine, their);
        if (choice == 2)
        {
            if (Take(theirs, theirSize, localSize, closeSave))
            {
                WriteSync(srv.hash);
                sAllowSend = true;
            }
            changed = true;
        }
        else if (choice == 1)
        {
            /* (over the server's: it keeps that one by date) */
            TandemNet_Log("saves: chose this PC's save");
            sAllowSend = true;
            Send(local, localSize, srv.hash);
        }
        else
            SetNote("Tandem: no save chosen - this PC's is played and not sent; restart the game to choose");
    }
    free(local);
    free(theirs);
    return changed;
}

/* ── while playing ────────────────────────────────────────────────────── */

/* connected after the start (the server was down then), or again after the connection broke */
static void Connected(const TandemServerSave *srv)
{
    unsigned size;
    uint8_t *local = ReadWhole(sPath, &size);
    bool real = RealSave(local, size);
    uint64_t hash = real ? TandemNet_Hash(local, size) : 0;

    if (srv->has && real && hash == srv->hash)
    {
        WriteSync(hash);
        sAllowSend = true;
        SetNote("");
    }
    else if (!srv->has || (sSyncHas && srv->hash == sSynced))
    {
        /* (the server's is the one this game knows: this PC's goes over it if it changed) */
        sAllowSend = true;
        SetNote("");
        if (real && (!sSyncHas || hash != sSynced))
            Send(local, size, srv->has ? srv->hash : 0);
    }
    else
    {
        sAllowSend = false;
        SetNote("Tandem: your save on the server is different (played on another PC) - restart the game to choose "
                "which save to play");
    }
    free(local);
}

void TandemSave_Update(void)
{
    TandemServerSave srv;
    unsigned long long stored;
    int r;
    uint32_t touched;

    if (!sJoining)
        return;
    TandemNet_GetServerSave(&srv);
    if (sPutInFlight && (r = TandemNet_SavePutResult(&stored)) != 0)
    {
        sPutInFlight = false;
        if (r == 1 && stored == sPutHash)
        {
            WriteSync(stored);
            TandemNet_Log("saves: the server has this PC's save now");
        }
        else if (srv.kind == 2)
        {
            /* (refused while connected: the server's changed in between) */
            sAllowSend = false;
            SetNote("Tandem: the server didn't take your save (its own changed) - restart the game to choose");
        }
        /* (else the connection broke: decided again when it's back) */
    }
    if (srv.kind == 2 && srv.connection != sHandledConnection && !sPutInFlight)
    {
        sHandledConnection = srv.connection;
        Connected(&srv);
    }
    touched = (uint32_t)SDL_AtomicGet(&sTouchedAt);
    if (touched != 0 && SDL_GetTicks() - touched >= SETTLE_MS && srv.kind == 2 && sAllowSend && !sPutInFlight
        && srv.connection == sHandledConnection)
    {
        unsigned size;
        uint8_t *local;

        SDL_AtomicCAS(&sTouchedAt, (int)touched, 0);
        local = ReadWhole(sPath, &size);
        if (RealSave(local, size) && (!sSyncHas || TandemNet_Hash(local, size) != sSynced))
            Send(local, size, sSyncHas ? sSynced : 0);
        free(local);
    }
}

void TandemSave_Quit(void)
{
    uint32_t start = SDL_GetTicks();
    TandemServerSave srv;

    if (!sJoining)
        return;
    /* (saved and closed at once: not settled yet - sent now; the game has stopped writing) */
    if (SDL_AtomicGet(&sTouchedAt) != 0)
    {
        uint32_t past = SDL_GetTicks() - SETTLE_MS;
        SDL_AtomicSet(&sTouchedAt, (int)(past ? past : 1));
    }
    for (;;)
    {
        TandemSave_Update();
        TandemNet_GetServerSave(&srv);
        if (srv.kind != 2 || !sAllowSend || (!sPutInFlight && SDL_AtomicGet(&sTouchedAt) == 0)
            || SDL_GetTicks() - start >= QUIT_WAIT_MS)
            break;
        SDL_Delay(20);
    }
    if (sPutInFlight)
        TandemNet_Log("saves: closed before the server had the last save - it's sent at the next start");
}
