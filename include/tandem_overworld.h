#ifndef GUARD_TANDEM_OVERWORLD_H
#define GUARD_TANDEM_OVERWORLD_H

/* Project Tandem T2 (src/tandem_overworld.c): friends drawn as characters ("ghosts") on my map */

/* local ids of the ghosts, one per player number (245-247: free - the link-cable rooms' players take 240-244
 * (OBJ_EVENT_ID_DYNAMIC_BASE + player), the overworld encounters start at 249) */
#define TANDEM_GHOST_LOCALID(slot) (245 + (slot))
/* local ids of their following Pokemon (232-234: free - map people use 1-64, the camera 127, the Berry Blender's
 * players start at 236) */
#define TANDEM_FOLLOWER_LOCALID(slot) (232 + (slot))
/* a friend or their Pokemon */
#define TANDEM_IS_GHOST_LOCALID(id) (((id) >= TANDEM_GHOST_LOCALID(0) && (id) <= TANDEM_GHOST_LOCALID(2)) \
                                  || ((id) >= TANDEM_FOLLOWER_LOCALID(0) && (id) <= TANDEM_FOLLOWER_LOCALID(2)))

/* once an overworld frame; does nothing unless connected */
void TandemOverworld_Update(void);
/* TRUE for a friend's ghost: blocks nobody, nobody blocks it, kept out of the saved slots */
bool8 TandemOverworld_IsGhost(const struct ObjectEvent *objectEvent);

#endif
