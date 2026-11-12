#ifndef WJ_VOXEL_COMPAT_3DS_LOG_H
#define WJ_VOXEL_COMPAT_3DS_LOG_H
#include "port_log.h"
#define CTR_LOG_VIDEO 0
#define CtrLog_Write(channel, ...) Voxel3D_Log(__VA_ARGS__)
#endif
