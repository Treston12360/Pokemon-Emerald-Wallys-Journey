#ifndef GUARD_PLATFORM_NULL_GUARD_H
#define GUARD_PLATFORM_NULL_GUARD_H

// PC build only: run a callback, skipping it if it reads through NULL
// (see src/platform/null_guard.c). Returns FALSE if it was skipped.
struct Sprite;
void PcGuard_Init(void);
bool32 PcGuard_Call0(void (*fn)(void));
bool32 PcGuard_CallSprite(void (*fn)(struct Sprite *), struct Sprite *sprite);
bool32 PcGuard_CallTask(void (*fn)(u8), u8 taskId);

#endif
