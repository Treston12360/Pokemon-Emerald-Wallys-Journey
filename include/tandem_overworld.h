#ifndef GUARD_TANDEM_OVERWORLD_H
#define GUARD_TANDEM_OVERWORLD_H

/* Project Tandem T2 (src/tandem_overworld.c): friends drawn as characters ("ghosts") on my map */

/* local ids of the ghosts, one per player number (245-247: free - the link-cable rooms' players take 240-244
 * (OBJ_EVENT_ID_DYNAMIC_BASE + player), the overworld encounters start at 249) */
#define TANDEM_GHOST_LOCALID(slot) (245 + (slot))

/* once an overworld frame; does nothing unless connected */
void TandemOverworld_Update(void);
/* TRUE for a friend's ghost: blocks nobody, nobody blocks it, kept out of the saved slots */
bool8 TandemOverworld_IsGhost(const struct ObjectEvent *objectEvent);

#endif
