#ifndef WJ_VOXEL_COMPAT_3DS_PLATFORM_H
#define WJ_VOXEL_COMPAT_3DS_PLATFORM_H
#include <stdbool.h>
#include <stddef.h>
/* The 3D camera's angle (degrees) and zoom (percent): PC settings (voxel3d_pc.c). */
int CtrSettings_VoxelPitch(void);
int CtrSettings_VoxelZoom(void);
bool CtrSettings_Voxel(void);
#endif
