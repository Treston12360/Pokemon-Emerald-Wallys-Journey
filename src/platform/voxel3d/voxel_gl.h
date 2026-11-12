#ifndef WJ_VOXEL_GL_H
#define WJ_VOXEL_GL_H
#include <stdbool.h>
// The 3D overworld (voxel_gl.c). Init after the GL context exists, from the folder holding voxel/.
bool Voxel3D_Init(const char *dataDir);
// Draws the 3D world into GL texture `colorTexture` (w x h). False: nothing drawn (not a map).
bool Voxel3D_Render(unsigned colorTexture, int w, int h);
unsigned Voxel3D_TextureId(void *sdlTexture);
void Voxel3D_SetSettings(bool on, int pitch, int zoom);
#endif
