#ifndef GUARD_ROTATING_GATE_H
#define GUARD_ROTATING_GATE_H

void RotatingGatePuzzleCameraUpdate(s16 deltaX, s16 deltaY);
void RotatingGate_InitPuzzleAndGraphics(void);
bool32 CheckForRotatingGatePuzzleCollision(enum Direction direction, s16 x, s16 y);
bool32 CheckForRotatingGatePuzzleCollisionWithoutAnimation(enum Direction direction, s16 x, s16 y);
#ifdef PORTABLE
// PC 3D view: whether `sprite` is a rotating gate - its pivot (map cells), its arms as laid at 0 degrees (8 bytes:
// north, east, south, west, each its first and second cell) and its turn now, clockwise: its cosine and sine (x 256)
bool8 RotatingGate_3DInfo(const struct Sprite *sprite, s16 *x, s16 *y, u8 *arms, s16 *cosine, s16 *sine);
#endif

#endif // GUARD_ROTATING_GATE_H
