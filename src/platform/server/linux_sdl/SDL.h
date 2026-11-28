/*
 * Project Tandem: the few SDL calls the server uses (src/platform/tandem_net.c), on plain POSIX threads - so the Linux
 * server (WallysJourneyServer-linux, for a cloud computer) is one file with nothing to install. Only that build uses
 * this header (Makefile.pc, tandem-linux-server); the game and the Windows server use real SDL.
 */
#ifndef TANDEM_LINUX_SDL_SHIM_H
#define TANDEM_LINUX_SDL_SHIM_H
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct { pthread_t t; } SDL_Thread;
typedef pthread_mutex_t SDL_mutex;
typedef struct { volatile int value; } SDL_atomic_t;
typedef int (*SDL_ThreadFunction)(void *);

struct TandemShimStart { SDL_ThreadFunction fn; void *data; };

static void *TandemShimRun(void *p)
{
    struct TandemShimStart s = *(struct TandemShimStart *)p;
    free(p);
    return (void *)(intptr_t)s.fn(s.data);
}

static inline SDL_Thread *SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    SDL_Thread *t = malloc(sizeof(*t));
    struct TandemShimStart *s = malloc(sizeof(*s));
    (void)name;
    s->fn = fn;
    s->data = data;
    if (t == NULL || s == NULL || pthread_create(&t->t, NULL, TandemShimRun, s) != 0)
    {
        free(t);
        free(s);
        return NULL;
    }
    return t;
}

static inline void SDL_WaitThread(SDL_Thread *t, int *status)
{
    void *r = NULL;
    if (t == NULL)
        return;
    pthread_join(t->t, &r);
    if (status != NULL)
        *status = (int)(intptr_t)r;
    free(t);
}

static inline SDL_mutex *SDL_CreateMutex(void)
{
    SDL_mutex *m = malloc(sizeof(*m));
    if (m != NULL)
        pthread_mutex_init(m, NULL);
    return m;
}
static inline int SDL_LockMutex(SDL_mutex *m) { return pthread_mutex_lock(m); }
static inline int SDL_UnlockMutex(SDL_mutex *m) { return pthread_mutex_unlock(m); }

static inline int SDL_AtomicGet(SDL_atomic_t *a) { return __atomic_load_n(&a->value, __ATOMIC_SEQ_CST); }
static inline int SDL_AtomicSet(SDL_atomic_t *a, int v) { return __atomic_exchange_n(&a->value, v, __ATOMIC_SEQ_CST); }

static inline uint64_t TandemShimNowNs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000u + (uint64_t)ts.tv_nsec;
}
static inline uint32_t SDL_GetTicks(void)
{
    static uint64_t start;
    if (start == 0)
        start = TandemShimNowNs();
    return (uint32_t)((TandemShimNowNs() - start) / 1000000u);
}
static inline uint64_t SDL_GetPerformanceCounter(void) { return TandemShimNowNs(); }
static inline void SDL_Delay(uint32_t ms) { usleep(ms * 1000u); }
static inline int SDL_Init(uint32_t flags) { (void)flags; return 0; }
static inline void SDL_SetMainReady(void) {}

#endif
