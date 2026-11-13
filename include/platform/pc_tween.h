#ifndef GUARD_PC_TWEEN_H
#define GUARD_PC_TWEEN_H

// DLX-3: in-between frames for battles and menus (src/platform/pc_tween.c).
void PcTween_BeginFrame(void);
bool32 PcTween_HasMotion(void);
void PcTween_Apply(float t);
void PcTween_End(void);

#endif
