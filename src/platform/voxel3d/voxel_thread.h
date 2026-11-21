#ifndef GUARD_VOXEL_THREAD_H
#define GUARD_VOXEL_THREAD_H
/*
 * Brennan 2026-11-21 02:12 (audit step 4, docs/polish_reports/PERF_AUDIT_2026-11-21.md: maps built on a worker
 * thread): the scenery builder keeps answers in caches it writes while it reads. Those are per thread
 * (VOXEL_TLS): the worker has its own, the main thread keeps its own, nothing is shared. Large ones are a per-thread
 * pointer to memory allocated on first use (VOXEL_TLS_TABLE), so a thread that never builds pays nothing.
 * The world itself (the maps placed) is voxel_world.c's VoxelWorldState, reached through a per-thread pointer that
 * is the main world on every thread except the worker, which points it at a snapshot.
 */
#include <stdlib.h>

#define VOXEL_TLS __thread

/* `type *name` = this thread's table of `count` entries, zeroed (NULL if out of memory) */
#define VOXEL_TLS_TABLE(type, name, count) \
    static VOXEL_TLS type *name##Tls_; \
    type *name = name##Tls_ != NULL ? name##Tls_ : (name##Tls_ = (type *)calloc((count), sizeof(type)))

#endif
