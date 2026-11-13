#ifndef WJ_VOXEL_GL_H
#define WJ_VOXEL_GL_H
#include <stdbool.h>
// The 3D overworld (voxel_gl.c). Init after the GL context exists, from the folder holding voxel/.
bool Voxel3D_Init(const char *dataDir);
// Draws the 3D world into GL texture `colorTexture` (w x h). False: nothing drawn (not a map).
bool Voxel3D_Render(unsigned colorTexture, int w, int h);
unsigned Voxel3D_TextureId(void *sdlTexture);
bool Voxel3D_ProjectPictureTile(float tileX, float tileY, float *outX, float *outY);
void Voxel3D_SetSettings(bool on, int pitch, int zoom);
/* DLX-5: draw the world as a battle's scenery (low battle camera on the spot the battle started, no people). */
void Voxel3D_SetBattle(bool on);
/* Gamma Emerald's lighting on or off (lays every map again). */
void Voxel3D_SetGammaLight(bool on);
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
