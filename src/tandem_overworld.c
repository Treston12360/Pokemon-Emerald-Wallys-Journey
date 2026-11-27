/*
 * Project Tandem T2 (docs/TANDEM_TASKS.md): friends walking on the same map, seen in real time.
 *
 * Once an overworld frame (OverworldBasic): my place goes to the network (src/platform/tandem_net.c), and each friend
 * on my map is drawn as a character - a "ghost": an object event in the PC's spare slots (16-63, never saved), with the
 * friend's own look (their outfit, bike, surfing), walking with the game's own movement actions from tile to tile as
 * their steps arrive, so the 2D and 3D views draw them as any character. Ghosts block nobody and nobody blocks them
 * (Brennan 2026-11-27: friends walk through each other - event_object_movement.c GetObjectObjectCollidesWith), have no
 * script and are never saved. A friend on another map is not drawn (T3: neighbouring maps, warps).
 *
 * Not connected: TandemOverworld_Update returns at once - single-player exactly as before.
 */
#include "global.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "sprite.h"
#include "tandem_overworld.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "platform/tandem_net.h"

/* steps waiting to be walked, per friend; more than this behind and the ghost jumps to the latest */
#define GHOST_QUEUE 16
#define GHOST_CATCH_UP 6

static TandemState sPending[TANDEM_MAX_PLAYERS][GHOST_QUEUE];
static u8 sPendingLen[TANDEM_MAX_PLAYERS];
static TandemState sLatest[TANDEM_MAX_PLAYERS];

/* (test aid: WJPC_TANDEM_DEBUG=1 - what the ghosts do, to the game's error output) */
#include <stdio.h>
extern char *getenv(const char *name);   /* (stdlib.h clashes with the game's macros) */
#define GHOST_LOG(...) do { static int sDbg = -1; if (sDbg < 0) sDbg = getenv("WJPC_TANDEM_DEBUG") != NULL; \
                            if (sDbg) fprintf(stderr, "[TandemGhost] " __VA_ARGS__); } while (0)

static struct ObjectEvent *FindGhost(unsigned slot)
{
    u32 i;

    for (i = SAVED_OBJECT_EVENTS_COUNT; i < OBJECT_EVENTS_COUNT; i++)
        if (gObjectEvents[i].active && gObjectEvents[i].localId == TANDEM_GHOST_LOCALID(slot))
            return &gObjectEvents[i];
    return NULL;
}

static void RemoveGhost(unsigned slot)
{
    struct ObjectEvent *ghost = FindGhost(slot);

    if (ghost != NULL)
    {
        GHOST_LOG("friend %u: removed\n", slot);
        RemoveObjectEvent(ghost);
    }
    sPendingLen[slot] = 0;
}

static u8 MySpeed(void)
{
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER))
        return 3;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE))
        return 2;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_DASH))
        return 1;
    return 0;
}

static void SendMyPlace(void)
{
    TandemState me;
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    memset(&me, 0, sizeof(me));
    if (player->active)
    {
        me.valid = 1;
        me.mapGroup = gSaveBlock1Ptr->location.mapGroup;
        me.mapNum = gSaveBlock1Ptr->location.mapNum;
        me.x = player->currentCoords.x;
        me.y = player->currentCoords.y;
        me.facing = player->facingDirection;
        me.speed = MySpeed();
        me.gfx = player->graphicsId;
        me.elevation = player->currentElevation;
    }
    TandemNet_SetMyState(&me);
}

static bool8 OnMyMap(const TandemState *st)
{
    return st->valid && st->mapGroup == gSaveBlock1Ptr->location.mapGroup && st->mapNum == gSaveBlock1Ptr->location.mapNum;
}

/* the next step of a ghost toward `to` (a tile it is at or next to; else it jumps there) */
static void StepGhost(struct ObjectEvent *ghost, const TandemState *to)
{
    s16 dx = to->x - ghost->currentCoords.x, dy = to->y - ghost->currentCoords.y;
    u32 dir;
    u8 action;

    ghost->currentElevation = ghost->previousElevation = to->elevation;
    GHOST_LOG("step %d,%d -> %d,%d facing %u speed %u\n", ghost->currentCoords.x, ghost->currentCoords.y, to->x, to->y,
              to->facing, to->speed);
    if (dx == 0 && dy == 0)
    {
        if (to->facing >= DIR_SOUTH && to->facing <= DIR_EAST && ghost->facingDirection != to->facing)
            ObjectEventSetHeldMovement(ghost, GetFaceDirectionMovementAction(to->facing));
        return;
    }
    dir = dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dy > 0 ? DIR_SOUTH : DIR_NORTH;
    if ((dx == 0 || dy == 0) && (dx * dx + dy * dy) == 1)
    {
        switch (to->speed)
        {
        case 1:  action = GetPlayerRunMovementAction(dir); break;
        case 2:  action = GetWalkFasterMovementAction(dir); break;
        case 3:  action = GetWalkFastMovementAction(dir); break;
        default: action = GetWalkNormalMovementAction(dir); break;
        }
        ObjectEventSetHeldMovement(ghost, action);
        return;
    }
    if ((dx == 0 || dy == 0) && (dx * dx + dy * dy) == 4)
    {
        /* (two tiles in a line: a ledge jump) */
        ObjectEventSetHeldMovement(ghost, GetJump2MovementAction(dir));
        return;
    }
    MoveObjectEventToMapCoords(ghost, to->x, to->y);
    ObjectEventTurn(ghost, to->facing >= DIR_SOUTH && to->facing <= DIR_EAST ? to->facing : DIR_SOUTH);
}

static u16 GhostGfx(u16 gfx)
{
    /* (test aid: WJPC_TANDEM_GHOST_GFX=N draws friends as graphics N) */
    static int sForce = -2;
    if (sForce == -2)
    {
        extern int atoi(const char *);
        const char *e = getenv("WJPC_TANDEM_GHOST_GFX");
        sForce = e != NULL ? atoi(e) : -1;
    }
    return sForce >= 0 ? (u16)sForce : gfx;
}

static void UpdateFriend(unsigned slot)
{
    TandemState st;
    struct ObjectEvent *ghost;

    /* everything that came in, in order: new tiles to walk; the latest place kept */
    while (TandemNet_PopState(slot, &st))
    {
        bool8 moved = sLatest[slot].x != st.x || sLatest[slot].y != st.y || sLatest[slot].mapNum != st.mapNum
                   || sLatest[slot].mapGroup != st.mapGroup;
        sLatest[slot] = st;
        if (!OnMyMap(&st))
            continue;
        if (moved || sPendingLen[slot] == 0)
        {
            if (sPendingLen[slot] == GHOST_QUEUE)
            {
                memmove(&sPending[slot][0], &sPending[slot][1], sizeof(TandemState) * (GHOST_QUEUE - 1));
                sPendingLen[slot]--;
            }
            sPending[slot][sPendingLen[slot]++] = st;
        }
        else
            sPending[slot][sPendingLen[slot] - 1] = st;   /* (the same tile: only its facing or look changed) */
    }
    if (!OnMyMap(&sLatest[slot]))
    {
        RemoveGhost(slot);
        return;
    }
    ghost = FindGhost(slot);
    if (ghost == NULL)
    {
        u8 id = SpawnSpecialObjectEventParameterized(GhostGfx(sLatest[slot].gfx), MOVEMENT_TYPE_NONE, TANDEM_GHOST_LOCALID(slot),
                                                     sLatest[slot].x, sLatest[slot].y, sLatest[slot].elevation);
        if (id >= OBJECT_EVENTS_COUNT)
            return;
        ghost = &gObjectEvents[id];
        GHOST_LOG("friend %u: drawn at %d,%d in slot %u (gfx %u)\n", slot, sLatest[slot].x, sLatest[slot].y, id, sLatest[slot].gfx);
        ObjectEventTurn(ghost, sLatest[slot].facing >= DIR_SOUTH && sLatest[slot].facing <= DIR_EAST ? sLatest[slot].facing : DIR_SOUTH);
        sPendingLen[slot] = 0;
        return;
    }
    {
        static u32 sTick;
        if (++sTick % 60 == 0)
            GHOST_LOG("friend %u: at %d,%d active %d invisible %d offscreen %d sprite %u spr.invisible %d gfx %u\n", slot,
                      ghost->currentCoords.x, ghost->currentCoords.y, ghost->active, ghost->invisible, ghost->offScreen,
                      ghost->spriteId, gSprites[ghost->spriteId].invisible, ghost->graphicsId);
        GHOST_LOG("   sprite inUse %d data0 %d (ghost slot %d) pos %d,%d+%d,%d coordOff %d oam.priority %d\n",
                  gSprites[ghost->spriteId].inUse, gSprites[ghost->spriteId].data[0], (int)(ghost - gObjectEvents),
                  gSprites[ghost->spriteId].x, gSprites[ghost->spriteId].y, gSprites[ghost->spriteId].x2,
                  gSprites[ghost->spriteId].y2, gSprites[ghost->spriteId].coordOffsetEnabled, gSprites[ghost->spriteId].oam.priority);
    }
    if (ghost->graphicsId != GhostGfx(sLatest[slot].gfx) && !ObjectEventIsMovementOverridden(ghost))
        ObjectEventSetGraphicsId(ghost, GhostGfx(sLatest[slot].gfx));
    /* (far behind: straight to the latest place) */
    if (sPendingLen[slot] > GHOST_CATCH_UP)
    {
        ObjectEventClearHeldMovement(ghost);
        MoveObjectEventToMapCoords(ghost, sLatest[slot].x, sLatest[slot].y);
        sPendingLen[slot] = 0;
        return;
    }
    if (ObjectEventIsMovementOverridden(ghost) && !ObjectEventClearHeldMovementIfFinished(ghost))
        return;   /* (still walking the last step) */
    if (sPendingLen[slot] > 0)
    {
        st = sPending[slot][0];
        memmove(&sPending[slot][0], &sPending[slot][1], sizeof(TandemState) * (GHOST_QUEUE - 1));
        sPendingLen[slot]--;
        StepGhost(ghost, &st);
    }
}

void TandemOverworld_Update(void)
{
    unsigned slot;

    if (!TandemNet_Active())
        return;
    SendMyPlace();
    for (slot = 0; slot < TANDEM_MAX_PLAYERS; slot++)
    {
        if (slot == TandemNet_MySlot())
            continue;
        if (!TandemNet_SlotConnected(slot))
        {
            if (sLatest[slot].valid)
            {
                RemoveGhost(slot);
                memset(&sLatest[slot], 0, sizeof(sLatest[slot]));
            }
            continue;
        }
        UpdateFriend(slot);
    }
}

bool8 TandemOverworld_IsGhost(const struct ObjectEvent *objectEvent)
{
    return objectEvent->localId >= TANDEM_GHOST_LOCALID(0) && objectEvent->localId <= TANDEM_GHOST_LOCALID(TANDEM_MAX_PLAYERS - 1);
}
