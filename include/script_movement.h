#ifndef GUARD_SCRIPT_MOVEMENT_H
#define GUARD_SCRIPT_MOVEMENT_H

// How many objects can follow a movement script at once. The list lives in a task's data
// (16 object ids and a 16-bit "finished" mask), so it stays 16 even where the PC build has
// more object-event slots (OBJECT_EVENTS_COUNT 64). On the GBA it equals OBJECT_EVENTS_COUNT.
#define SCRIPT_MOVEMENT_SLOTS 16

bool8 ScriptMovement_StartObjectMovementScript(u8 localId, u8 mapNum, u8 mapGroup, const u8 *movementScript);
bool8 ScriptMovement_IsObjectMovementFinished(u8 localId, u8 mapNum, u8 mapGroup);
bool32 ScriptMovement_IsAllObjectMovementFinished(void);
void ScriptMovement_UnfreezeObjectEvents(void);

#endif // GUARD_SCRIPT_MOVEMENT_H
