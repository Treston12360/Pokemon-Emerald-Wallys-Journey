#ifndef WJ_VOXEL_GL_H
#define WJ_VOXEL_GL_H
#include <stdbool.h>
// The 3D overworld (voxel_gl.c). Init after the GL context exists, from the folder holding voxel/.
bool Voxel3D_Init(const char *dataDir);
// Draws the 3D world into GL texture `colorTexture` (w x h). False: nothing drawn (not a map).
bool Voxel3D_Render(unsigned colorTexture, int w, int h);
/* Audit step 5 (240 fps): Render only updates the world (no drawing) while on; the refreshes then draw it with
 * Voxel3D_Redraw, eased `t` (0..1) from the game frame before to the last one. */
void Voxel3D_SetUpdateOnly(bool on);
/* game frames since the last drawn one (fast forward draws one of several): the eased moves span that many */
void Voxel3D_SetDrawSpan(unsigned frames);
bool Voxel3D_Redraw(unsigned colorTexture, int w, int h, float t);
unsigned Voxel3D_TextureId(void *sdlTexture);
bool Voxel3D_ProjectPictureTile(float tileX, float tileY, float *outX, float *outY);
void Voxel3D_SetSettings(bool on, int pitch, int zoom);
/* DLX-5: draw the world as a battle's scenery (low battle camera on the spot the battle started, no people). */
void Voxel3D_SetBattle(bool on);
/* Gamma Emerald's lighting on or off (lays every map again). */
void Voxel3D_SetGammaLight(bool on);
/* Polish option 5: anti-aliasing samples a pixel (0/1 off, 2, 4, 8; Gamma lighting's path). */
void Voxel3D_SetAntiAlias(int samples);
/* Effects switched off live (F1-F6 in sdl2.c; Brennan 2026-11-21 10:34): a bit each. */
#define VOXEL_FX_CLOUDS     1u  /* F1: cloud shadows */
#define VOXEL_FX_BLOOM      2u  /* F2: glow */
#define VOXEL_FX_ANTIALIAS  4u  /* F3: anti-aliasing */
#define VOXEL_FX_REFRESH    8u  /* F4: 3D drawn every refresh (off: once a game frame, 60 a second) */
#define VOXEL_FX_DOF       16u  /* F5: depth blur */
#define VOXEL_FX_CRACKFILL 32u  /* F6: the final pass's crack fill */
#define VOXEL_FX_GAPS      64u  /* F7 (on, not off): the 3D background magenta, so a gap through the ground shows */
/* Brennan 2026-11-21 15:33 (21i, F7: the black lines stay black, so something draws them): one kind of 3D piece left
 * out at a time, to find which draws the lines (and which strobes under the trees) */
#define VOXEL_FX_GROUND   128u  /* F8: the ground of the maps (their own terrain) */
#define VOXEL_FX_TREES    256u  /* F9: the trees */
#define VOXEL_FX_MODELS   512u  /* F10: models - fences, buildings, signs, rocks */
#define VOXEL_FX_RING    1024u  /* F12: the ring of far ground round the loaded maps (the "belt"), its trees too */
extern unsigned gVoxelFxOff;
/* Emerald 3DS's battle frame: the intro is sliding its scenery in (the camera glides then), and BG3's scroll
 * from rest in GBA pixels (a move shaking the scenery; 0 while sliding or with a move's background). */
void Voxel3D_SetBattleFrame(bool introSliding, float shakeX, float shakeY);
/* The battle entrance's effects (Emerald 3DS's CtrVoxelBattleIntro): flight 0..1 (how high the camera
 * climbs), speed 0..1 (how fast it travels), impact (the landing), bars (cinema bars), time (frames). */
typedef struct
{
    float flight, speed, impact, bars, time;
} Voxel3DBattleIntro;
void Voxel3D_BattleIntro(Voxel3DBattleIntro *intro);
#endif
