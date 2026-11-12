#ifndef WJ_VOXEL_COMPAT_3DS_FIELD_VIEW_H
#define WJ_VOXEL_COMPAT_3DS_FIELD_VIEW_H
#define CTR_FIELD_EDGE_LOW 1u
#define CTR_FIELD_EDGE_HIGH 2u
#define CTR_FIELD_EDGE_WEST CTR_FIELD_EDGE_LOW
#define CTR_FIELD_EDGE_EAST CTR_FIELD_EDGE_HIGH
#define CTR_FIELD_EDGE_NORTH (CTR_FIELD_EDGE_LOW << 2)
#define CTR_FIELD_EDGE_SOUTH (CTR_FIELD_EDGE_HIGH << 2)
float CtrFieldView_Limit(float at, float before, float after, float low, float high, unsigned closed);
void CtrFieldView_SetVoxelShift(int x, int y);
#endif
