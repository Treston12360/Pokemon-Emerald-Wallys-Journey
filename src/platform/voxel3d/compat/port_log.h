#ifndef WJ_VOXEL_COMPAT_PORT_LOG_H
#define WJ_VOXEL_COMPAT_PORT_LOG_H
void Voxel3D_Log(const char *fmt, ...);
#define PORT_LOG(...) Voxel3D_Log(__VA_ARGS__)
#endif
