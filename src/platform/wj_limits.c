// PC only: the GBA limits on people (object events), sprites and sprite colours, raised.
#include "global.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"

// The GBA has 16 object-event slots and the save keeps exactly those 16 (so mGBA saves load).
// The PC has OBJECT_EVENTS_COUNT live slots. Slots 16 and up are not saved: after loading,
// the people in them simply reload from the map.
//  - The player and both followers only ever use the saved slots (the NPC follower's slot number
//    is itself saved), exactly as on the GBA.
//  - Other people take the lowest free slot below 14, so the ones you meet are saved where
//    they stand; the last two saved slots are left for a follower while there is room higher up.
#define RESERVED_FOR_FOLLOWERS 2

u32 WJPC_PickObjectEventSlot(u32 firstFree, u16 localId)
{
    u32 i;

    if (firstFree >= OBJECT_EVENTS_COUNT)
        return firstFree;
    if (localId == OBJ_EVENT_ID_PLAYER || localId == OBJ_EVENT_ID_FOLLOWER || localId == OBJ_EVENT_ID_NPC_FOLLOWER)
        return firstFree < SAVED_OBJECT_EVENTS_COUNT ? firstFree : OBJECT_EVENTS_COUNT;
    if (firstFree < SAVED_OBJECT_EVENTS_COUNT - RESERVED_FOR_FOLLOWERS)
        return firstFree;
    for (i = SAVED_OBJECT_EVENTS_COUNT; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (!gObjectEvents[i].active)
            return i;
    }
    return firstFree;
}
