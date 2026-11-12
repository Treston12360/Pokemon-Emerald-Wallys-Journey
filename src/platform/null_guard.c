// PC build only. "GBA leniency" for reads through NULL.
//
// On the GBA, reading through a NULL pointer just reads the BIOS, so the game
// has many harmless bugs where a callback runs one frame after its data was
// freed (VBlank callbacks, sprite callbacks, tasks). On PC those reads crash.
// Calls made through PcGuard_* catch any bad memory access (a read or write
// through a NULL or junk pointer, as the GBA reads junk there), skip the rest of
// that one callback, and log it once (also to WallysJourney_errors.txt) so it can
// be fixed properly. On Windows, any other crash writes WallysJourney_crash.txt.
#ifndef _WIN32
#define _GNU_SOURCE // REG_EIP etc. in <sys/ucontext.h>
#include <ucontext.h>
#endif
#include <stdio.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "global.h"
#include "platform/null_guard.h"
#include "main.h"

#ifdef _WIN32
#else
#include <signal.h>
#ifdef __GLIBC__
#include <execinfo.h>
#endif
#include <string.h>
#endif

// Any bad memory access (or integer divide by zero) inside a guarded callback is skipped:
// the GBA never faults on a read or write, it reads junk / ignores the write and carries on.
static __thread void **tGuardBuf;
static __thread uintptr_t tFaultAddr;
static __thread unsigned tFaultCode;

#define MAX_LOGGED 64
static const void *sLogged[MAX_LOGGED];
static int sLoggedCount;

static void LogOnce(const void *fn)
{
    int i;
    FILE *f;
    for (i = 0; i < sLoggedCount; i++)
        if (sLogged[i] == fn)
            return;
    if (sLoggedCount < MAX_LOGGED)
        sLogged[sLoggedCount++] = fn;
    fprintf(stderr, "WJPC: skipped a bad memory access in callback %p (code %x, address %p; harmless on GBA)\n",
            fn, tFaultCode, (void *)tFaultAddr);
    fflush(stderr);
    // Also keep a file next to the game so the owner can send it.
    f = fopen("WallysJourney_errors.txt", "a");
    if (f != NULL)
    {
#ifdef _WIN32
        fprintf(f, "skipped: callback %p (exe+%lx), code %x, address %p, callback2 exe+%lx\n", fn,
                (unsigned long)((uintptr_t)fn - (uintptr_t)GetModuleHandleA(NULL)), tFaultCode, (void *)tFaultAddr,
                (unsigned long)((uintptr_t)gMain.callback2 - (uintptr_t)GetModuleHandleA(NULL)));
#else
        fprintf(f, "skipped: callback %p, code %x, address %p, callback2 %p\n", fn, tFaultCode,
                (void *)tFaultAddr, (void *)gMain.callback2);
#endif
        fclose(f);
    }
}

// Integer divide by zero, the GBA way. This ROM's division routines (libgcc) give for x / 0 the
// biggest number with x's sign (0 if x is 0; unsigned: 0xFFFFFFFF, or 0) and for x % 0 the value 0,
// and the game carries on. x86 stops instead, so the faulting DIV/IDIV r/m32 is done here by hand:
// quotient into EAX, remainder 0 into EDX, then continue after the instruction.
// Returns the instruction length, or 0 if it isn't a 32-bit DIV/IDIV (then it's handled as a crash).
static int EmulateDivideByZero(const uint8_t *ip, uint32_t *eax, uint32_t *edx)
{
    const uint8_t *p = ip;
    int mod, reg, rm;
    if (*p == 0x66 || *p == 0xF6)
        return 0; // 16/8-bit forms: not used by this game's C code
    if (*p != 0xF7)
        return 0;
    p++;
    mod = *p >> 6;
    reg = (*p >> 3) & 7;
    rm = *p & 7;
    p++;
    if (reg != 6 && reg != 7) // /6 = DIV, /7 = IDIV
        return 0;
    if (mod != 3 && rm == 4)
    {
        // SIB byte; base 5 with mod 0 means a 32-bit displacement
        if (mod == 0 && (*p & 7) == 5)
            p += 4;
        p++;
    }
    if (mod == 1)
        p += 1;
    else if (mod == 2 || (mod == 0 && rm == 5))
        p += 4;
    if (reg == 7)
    {
        int32_t x = (int32_t)*eax;
        *eax = x > 0 ? 0x7FFFFFFF : x < 0 ? 0x80000000 : 0;
    }
    else
    {
        *eax = *eax != 0 ? 0xFFFFFFFF : 0;
    }
    *edx = 0;
    return (int)(p - ip);
}

#ifdef _WIN32
static LONG CALLBACK Handler(EXCEPTION_POINTERS *info)
{
    DWORD code = info->ExceptionRecord->ExceptionCode;
#ifdef _X86_
    if (code == EXCEPTION_INT_DIVIDE_BY_ZERO)
    {
        uint32_t eax = info->ContextRecord->Eax, edx = info->ContextRecord->Edx;
        int len = EmulateDivideByZero((const uint8_t *)info->ContextRecord->Eip, &eax, &edx);
        if (len > 0)
        {
            info->ContextRecord->Eax = eax;
            info->ContextRecord->Edx = edx;
            info->ContextRecord->Eip += len;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
#endif
    if (tGuardBuf != NULL
     && (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_INT_DIVIDE_BY_ZERO
      || code == EXCEPTION_INT_OVERFLOW || code == EXCEPTION_ILLEGAL_INSTRUCTION
      || code == EXCEPTION_ARRAY_BOUNDS_EXCEEDED || code == EXCEPTION_IN_PAGE_ERROR))
    {
        tFaultCode = code;
        tFaultAddr = info->ExceptionRecord->NumberParameters >= 2
                   ? info->ExceptionRecord->ExceptionInformation[1]
                   : (uintptr_t)info->ExceptionRecord->ExceptionAddress;
        __builtin_longjmp(tGuardBuf, 1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// A crash nothing caught: write WallysJourney_crash.txt (so the owner can send it) and say so.
static LONG WINAPI CrashReport(EXCEPTION_POINTERS *info)
{
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
    uintptr_t at = (uintptr_t)info->ExceptionRecord->ExceptionAddress;
    FILE *f = fopen("WallysJourney_crash.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "Wally's Journey PC crash report\n");
        fprintf(f, "code %lx at %p (exe+%lx)\n", (unsigned long)info->ExceptionRecord->ExceptionCode,
                (void *)at, (unsigned long)(at - base));
        if (info->ExceptionRecord->NumberParameters >= 2)
            fprintf(f, "access %lu to %p\n", (unsigned long)info->ExceptionRecord->ExceptionInformation[0],
                    (void *)info->ExceptionRecord->ExceptionInformation[1]);
        fprintf(f, "callback1 exe+%lx callback2 exe+%lx\n",
                (unsigned long)((uintptr_t)gMain.callback1 - base), (unsigned long)((uintptr_t)gMain.callback2 - base));
#ifdef _X86_
        {
            // Return addresses on the stack, for finding the caller.
            uintptr_t *sp = (uintptr_t *)info->ContextRecord->Esp;
            int i, n = 0;
            fprintf(f, "eip exe+%lx ebp %lx\nstack:", (unsigned long)(info->ContextRecord->Eip - base),
                    (unsigned long)info->ContextRecord->Ebp);
            for (i = 0; i < 2048 && n < 40; i++)
            {
                MEMORY_BASIC_INFORMATION mbi;
                if (i % 1024 == 0 && VirtualQuery(sp + i, &mbi, sizeof(mbi)) == 0)
                    break;
                if (sp[i] > base && sp[i] < base + 0x2000000)
                {
                    fprintf(f, " exe+%lx", (unsigned long)(sp[i] - base));
                    n++;
                }
            }
            fprintf(f, "\n");
        }
#endif
        fclose(f);
    }
    MessageBoxA(NULL, "Wally's Journey crashed.\n\nA file called WallysJourney_crash.txt was saved next to the game. "
                      "Please send it so the crash can be fixed.", "Wally's Journey", MB_OK | MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
static void Handler(int sig, siginfo_t *si, void *ctx)
{
#if defined(__i386__) && defined(REG_EIP)
    if (sig == SIGFPE && si->si_code == FPE_INTDIV)
    {
        ucontext_t *uc = ctx;
        uint32_t eax = uc->uc_mcontext.gregs[REG_EAX], edx = uc->uc_mcontext.gregs[REG_EDX];
        int len = EmulateDivideByZero((const uint8_t *)uc->uc_mcontext.gregs[REG_EIP], &eax, &edx);
        if (len > 0)
        {
            uc->uc_mcontext.gregs[REG_EAX] = eax;
            uc->uc_mcontext.gregs[REG_EDX] = edx;
            uc->uc_mcontext.gregs[REG_EIP] += len;
            return;
        }
    }
#endif
    if (tGuardBuf != NULL)
    {
        tFaultCode = sig;
        tFaultAddr = (uintptr_t)si->si_addr;
        __builtin_longjmp(tGuardBuf, 1);
    }
    // Not ours: report where, then crash normally.
    {
        fprintf(stderr, "WJPC: crash, bad address %p\n", si->si_addr);
#ifdef __GLIBC__
        void *frames[32];
        int n = backtrace(frames, 32);
        backtrace_symbols_fd(frames, n, 2);
#endif
    }
    signal(sig, SIG_DFL);
}
#endif

void PcGuard_Init(void)
{
#ifdef _WIN32
    AddVectoredExceptionHandler(1, Handler);
    SetUnhandledExceptionFilter(CrashReport);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = Handler;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
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
