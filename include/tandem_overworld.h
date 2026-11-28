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

/* T4: A on a ghost (field_control_avatar.c) - a friend's menu (data/scripts/tandem.inc), or NULL for their Pokemon */
const u8 *TandemOverworld_GhostScript(const struct ObjectEvent *objectEvent);
/* T4: TRUE while a friend's card is shown (trainer_card.c): `badgeCount` filled with their badges, not mine */
bool8 TandemOverworld_FriendCardBadges(u8 *badgeCount);
/* T4: friends in their own outfits - their sprites have their own graphics ids (never the Brendan / May ids my
 * outfit replaces) and palettes (OBJ_EVENT_PAL_TAG_TANDEM_FRIEND + slot) */
extern u16 gTandemFriendOwPal[3][16];
bool8 TandemOverworld_IsGhostGfx(u16 graphicsId);
/* a friend's sprite: their outfit's, or NULL with *graphicsId set to their plain sprite */
const struct ObjectEventGraphicsInfo *TandemOverworld_GhostGfxInfo(u16 *graphicsId);
/* TRUE while a friend's card is shown; and whether it shows their outfit (TRAINER_PIC_WJ_PLAYER, theirs for now) */
bool8 TandemOverworld_ViewingFriendCard(void);
bool8 TandemOverworld_FriendCardOutfit(void);
/* specials (data/specials.inc): gStringVar1 = the friend's name, VAR_0x8004 = whether their card has come; their card */
void Tandem_BufferFriendName(void);
/* the friend talked to (or who asked me to trade) */
u8 TandemOverworld_TalkSlot(void);
void TandemOverworld_SetTalkSlot(u8 slot);
void Tandem_ShowFriendCard(void);

#endif
