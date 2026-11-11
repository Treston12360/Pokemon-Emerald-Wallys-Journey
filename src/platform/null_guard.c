// PC build only. "GBA leniency" for reads through NULL.
//
// On the GBA, reading through a NULL pointer just reads the BIOS, so the game
// has many harmless bugs where a callback runs one frame after its data was
// freed (VBlank callbacks, sprite callbacks, tasks). On PC those reads crash.
// Calls made through PcGuard_* catch a fault on the first 64 KB of memory (a
// read or write through a NULL struct pointer), skip the rest of that one
// callback, and log it once so it can be fixed properly. Any other crash is
// left alone.
#include <stdio.h>
#include <stdint.h>
#include "global.h"
#include "platform/null_guard.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <execinfo.h>
#include <string.h>
#endif

#define LOW_ADDR_LIMIT 0x10000

static __thread void **tGuardBuf;

#define MAX_LOGGED 64
static const void *sLogged[MAX_LOGGED];
static int sLoggedCount;

static void LogOnce(const void *fn)
{
    int i;
    for (i = 0; i < sLoggedCount; i++)
        if (sLogged[i] == fn)
            return;
    if (sLoggedCount < MAX_LOGGED)
        sLogged[sLoggedCount++] = fn;
    fprintf(stderr, "WJPC: skipped a NULL read in callback %p (harmless on GBA)\n", fn);
    fflush(stderr);
}

#ifdef _WIN32
static LONG CALLBACK Handler(EXCEPTION_POINTERS *info)
{
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
     && tGuardBuf != NULL
     && info->ExceptionRecord->NumberParameters >= 2
     && info->ExceptionRecord->ExceptionInformation[1] < LOW_ADDR_LIMIT)
    {
        __builtin_longjmp(tGuardBuf, 1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#else
static void Handler(int sig, siginfo_t *si, void *ctx)
{
    if (tGuardBuf != NULL && (uintptr_t)si->si_addr < LOW_ADDR_LIMIT)
        __builtin_longjmp(tGuardBuf, 1);
    // Not ours: report where, then crash normally.
    {
        void *frames[32];
        int n = backtrace(frames, 32);
        fprintf(stderr, "WJPC: crash, bad address %p\n", si->si_addr);
        backtrace_symbols_fd(frames, n, 2);
    }
    signal(sig, SIG_DFL);
}
#endif

void PcGuard_Init(void)
{
#ifdef _WIN32
    AddVectoredExceptionHandler(1, Handler);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = Handler;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
#endif
}

// Each wrapper keeps its own jump buffer so guards can nest.
#define GUARDED(fnptr, call)                      \
    void *buf[5];                                 \
    void **prev = tGuardBuf;                      \
    if (__builtin_setjmp(buf) == 0)               \
    {                                             \
        tGuardBuf = buf;                          \
        call;                                     \
        tGuardBuf = prev;                         \
        return TRUE;                              \
    }                                             \
    tGuardBuf = prev;                             \
    LogOnce((const void *)(fnptr));               \
    return FALSE;

__attribute__((noinline)) bool32 PcGuard_Call0(void (*fn)(void))
{
    GUARDED(fn, fn())
}

__attribute__((noinline)) bool32 PcGuard_CallSprite(void (*fn)(struct Sprite *), struct Sprite *sprite)
{
    GUARDED(fn, fn(sprite))
}

__attribute__((noinline)) bool32 PcGuard_CallTask(void (*fn)(u8), u8 taskId)
{
    GUARDED(fn, fn(taskId))
}
