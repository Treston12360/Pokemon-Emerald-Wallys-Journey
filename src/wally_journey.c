// Wally's Journey: persistent progression and observed-only rival memory.
// Uses saved Emerald variable IDs; does not inspect the player's hidden party.
#include "global.h"
#include "pokedex.h"
#include "main.h"
#include "palette.h"
#include "script.h"
#include "event_data.h"
#include "wally_journey.h"
#include "constants/items.h"
#include "constants/game_stat.h"
#include "constants/moves.h"
#include "pokemon.h"
#include "rtc.h"
#include "battle.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "constants/contest.h"
#include "constants/weather.h"
#include "constants/map_types.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "field_weather.h"
#include "overworld.h"
#include "string_util.h"
#include "decompress.h"
#include "trig.h"
#include "sprite.h"
#include "fieldmap.h"
#include "global.fieldmap.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "metatile_behavior.h"
#include "script_movement.h"
#include "constants/event_object_movement.h"
#include "constants/hold_effects.h"
#include "constants/field_effects.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"

#define WALLY_VAR_PEAK_BADGES   VAR_UNUSED_0x40F7
#define WALLY_VAR_BIOME_MASK    VAR_UNUSED_0x40F8
#define WALLY_VAR_WALLY_DEFEATS VAR_UNUSED_0x40F9
#define WALLY_VAR_WALLY_WINS    VAR_UNUSED_0x40FA
#define WALLY_VAR_LAST_SPECIES  VAR_UNUSED_0x40FB
#define WALLY_VAR_PREV_SPECIES  VAR_UNUSED_0x40FC
#define WALLY_VAR_TITLE_DEFENSES VAR_UNUSED_0x40FD

enum WallyBiomeBit {
    WALLY_BIOME_FOREST       = 1 << 0,
    WALLY_BIOME_CAVE         = 1 << 1,
    WALLY_BIOME_CITY         = 1 << 2,
    WALLY_BIOME_DESERT       = 1 << 3,
    WALLY_BIOME_VOLCANO      = 1 << 4,
    WALLY_BIOME_RAIN         = 1 << 5,
    WALLY_BIOME_HOME         = 1 << 6,
    WALLY_BIOME_VICTORY_ROAD = 1 << 7
};
static u16 CountBits(u16 bits) {
    u16 count = 0;
    while (bits) { count += bits & 1; bits >>= 1; }
    return count;
}
static u16 CountBadges(void) {
    u16 count = 0;
    for (u16 i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i)) count++;
    return count;
}
static u16 AreaBit(u8 group,u8 num) {
#define MAP_IS(map) (group == MAP_GROUP(map) && num == MAP_NUM(map))
    if (MAP_IS(MAP_ROUTE104)) return WALLY_BIOME_FOREST;
    if (MAP_IS(MAP_GRANITE_CAVE_1F)) return WALLY_BIOME_CAVE;
    if (MAP_IS(MAP_MAUVILLE_CITY)) return WALLY_BIOME_CITY;
    if (MAP_IS(MAP_ROUTE111)) return WALLY_BIOME_DESERT;
    if (MAP_IS(MAP_MT_CHIMNEY)) return WALLY_BIOME_VOLCANO;
    if (MAP_IS(MAP_ROUTE119)) return WALLY_BIOME_RAIN;
    if (MAP_IS(MAP_VERDANTURF_TOWN)) return WALLY_BIOME_HOME;
    if (MAP_IS(MAP_VICTORY_ROAD_1F)) return WALLY_BIOME_VICTORY_ROAD;
#undef MAP_IS
    return 0;
}
void WallyJourney_Reset(void) {
    VarSet(WALLY_VAR_PEAK_BADGES,0);
    VarSet(WALLY_VAR_BIOME_MASK,0);
    VarSet(WALLY_VAR_WALLY_DEFEATS,0);
    VarSet(WALLY_VAR_WALLY_WINS,0);
    VarSet(WALLY_VAR_LAST_SPECIES,0);
    VarSet(WALLY_VAR_PREV_SPECIES,0);
    VarSet(WALLY_VAR_TITLE_DEFENSES,0);
#if !IS_FRLG
    FlagSet(FLAG_WALLY_EXP_ALL_MODE); // new games start with EXP. ALL (Options can switch to SHARE)
#endif
}
void WallyJourney_OnMapEntered(u8 group,u8 num) {
    u16 badges = CountBadges();
    u16 known = VarGet(WALLY_VAR_BIOME_MASK), bit = AreaBit(group,num);
    if (badges > VarGet(WALLY_VAR_PEAK_BADGES))
        VarSet(WALLY_VAR_PEAK_BADGES,badges);
    if (bit && !(known & bit))
        VarSet(WALLY_VAR_BIOME_MASK,known | bit);
    // National Dex from the start; also upgrades saves made before this change.
    if (FlagGet(FLAG_SYS_POKEDEX_GET) && !IsNationalPokedexEnabled())
        EnableNationalPokedex();
    WallyJourney_SyncExpItem();
}
u16 WallyJourney_GetPeakBadges(void) {
    u16 old = VarGet(WALLY_VAR_PEAK_BADGES), now = CountBadges();
    return old > now ? old : now;
}
u16 WallyJourney_GetDiscoveredBiomes(void) {
    return CountBits(VarGet(WALLY_VAR_BIOME_MASK) & 0xff);
}
u16 WallyJourney_GetSuggestedAceLevel(void) {
    u16 level = 5 + WallyJourney_GetPeakBadges() * 5 + WallyJourney_GetDiscoveredBiomes() * 2;
    return level > 60 ? 60 : level;
}
u16 WallyJourney_GetConfidence(void) {
    u16 score = 8 + WallyJourney_GetPeakBadges() * 8 + WallyJourney_GetDiscoveredBiomes() * 3;
    return score > 95 ? 95 : score;
}
void WallyJourney_RecordRivalBattle(u8 playerWon) {
    u16 var = playerWon ? WALLY_VAR_WALLY_DEFEATS : WALLY_VAR_WALLY_WINS;
    u16 count = VarGet(var);
    if (count != 0xffff) VarSet(var,count+1);
}
u16 WallyJourney_GetWallyDefeats(void) { return VarGet(WALLY_VAR_WALLY_DEFEATS); }
u16 WallyJourney_GetWallyVictories(void) { return VarGet(WALLY_VAR_WALLY_WINS); }
void WallyJourney_RecordObservedSpecies(u16 species) {
    u16 last,prev;
    if (species == SPECIES_NONE) return;
    last = VarGet(WALLY_VAR_LAST_SPECIES);
    prev = VarGet(WALLY_VAR_PREV_SPECIES);
    if (last == species) return;
    VarSet(WALLY_VAR_LAST_SPECIES,species);
    if (prev != last) VarSet(WALLY_VAR_PREV_SPECIES,last);
}
u16 WallyJourney_GetLastObservedSpecies(void) { return VarGet(WALLY_VAR_LAST_SPECIES); }
u16 WallyJourney_GetPreviousObservedSpecies(void) { return VarGet(WALLY_VAR_PREV_SPECIES); }
bool8 WallyJourney_ShouldLeadWithMagneton(u16 primary,u16 secondary) {
    if (primary == TYPE_GROUND || secondary == TYPE_GROUND) return FALSE;
    return primary == TYPE_WATER || secondary == TYPE_WATER
        || primary == TYPE_FLYING || secondary == TYPE_FLYING;
}

// Every capture ball except the standard Poké Ball. Safari Balls are left
// alone: the Safari Zone uses its own ball counter, not Bag items.
bool32 WallyJourney_IsForbiddenBall(u16 itemId)
{
    return itemId >= ITEM_GREAT_BALL && itemId <= ITEM_CHERISH_BALL
        && itemId != ITEM_SAFARI_BALL;
}
u16 WallyJourney_ConvertForbiddenBall(u16 itemId)
{
    // Engine battle tests give special balls on purpose; only the real game enforces the rule.
    if (TESTING)
        return itemId;
    return WallyJourney_IsForbiddenBall(itemId) ? ITEM_POKE_BALL : itemId;
}

bool32 WallyJourney_IsExpAllMode(void)
{
#if IS_FRLG
    return FALSE;
#else
    // The Options mode decides which item Devon's Exp. device is; EXP. ALL only
    // works while the player has the key item and has it switched on.
    return FlagGet(FLAG_WALLY_EXP_ALL_MODE) && FlagGet(FLAG_WALLY_EXP_ALL_ON)
        && CheckBagHasItem(ITEM_EXP_ALL, 1);
#endif
}

#if !IS_FRLG
// Take one Exp. Share away from the player: bag first, then a party Pokemon's
// held item, then a boxed Pokemon's held item.
static bool32 TakeExpShare(void)
{
    u32 i, j;
    u16 none = ITEM_NONE;
    if (CheckBagHasItem(ITEM_EXP_SHARE, 1))
        return RemoveBagItem(ITEM_EXP_SHARE, 1);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HELD_ITEM) == ITEM_EXP_SHARE)
        {
            SetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HELD_ITEM, &none);
            return TRUE;
        }
    }
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonDataAt(i, j, MON_DATA_HELD_ITEM) == ITEM_EXP_SHARE)
            {
                SetBoxMonDataAt(i, j, MON_DATA_HELD_ITEM, &none);
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif

// Wild catch counts per National Dex number. The magic byte marks the table as
// initialised so saves from before this feature start every species at zero.
#define WJ_CATCH_MAGIC 0xA7
static void EnsureCatchCounts(void)
{
    if (gSaveBlock3Ptr->wjCatchCountMagic != WJ_CATCH_MAGIC)
    {
        memset(gSaveBlock3Ptr->wjCatchCounts, 0, sizeof(gSaveBlock3Ptr->wjCatchCounts));
        gSaveBlock3Ptr->wjCatchCountMagic = WJ_CATCH_MAGIC;
    }
}

// Forms seen. A base form counts as seen when the Pokedex says so; any other form (regional
// forms, Morpeko's modes...) only once that exact form has been seen in battle or obtained.
#define WJ_FORM_SEEN_MAGIC 0x5F
static void EnsureFormSeen(void)
{
    if (gSaveBlock3Ptr->wjFormSeenMagic != WJ_FORM_SEEN_MAGIC)
    {
        memset(gSaveBlock3Ptr->wjFormSeen, 0, sizeof(gSaveBlock3Ptr->wjFormSeen));
        gSaveBlock3Ptr->wjFormSeenMagic = WJ_FORM_SEEN_MAGIC;
    }
}

void WallyJourney_SetFormSeen(u16 species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    EnsureFormSeen();
    gSaveBlock3Ptr->wjFormSeen[species / 8] |= 1 << (species % 8);
}

bool32 WallyJourney_FormSeen(u16 species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;
    EnsureFormSeen();
    if (gSaveBlock3Ptr->wjFormSeen[species / 8] & (1 << (species % 8)))
        return TRUE;
    if (GET_BASE_SPECIES_ID(species) == species)
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN);
    return FALSE;
}

void WallyJourney_RecordCatch(u16 species)
{
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    WallyJourney_SetFormSeen(species);
    EnsureCatchCounts();
    if (dexNum == 0 || dexNum > NATIONAL_DEX_COUNT)
        return;
    if (gSaveBlock3Ptr->wjCatchCounts[dexNum] < 255)
        gSaveBlock3Ptr->wjCatchCounts[dexNum]++;
}

// Shiny encounters (owner request 2026-10-29, shown on the trainer card back): every wild battle
// whose opponent is shiny adds 1 to GAME_STAT_WJ_SHINY_ENCOUNTERS, and the species is recorded
// (by National Dex number) in the list of shinies met. Called as each wild battle starts.
#define WJ_SHINY_SEEN_MAGIC 0x5A
static void EnsureShinySeen(void)
{
    if (gSaveBlock3Ptr->wjShinySeenMagic != WJ_SHINY_SEEN_MAGIC)
    {
        memset(gSaveBlock3Ptr->wjShinySeen, 0, sizeof(gSaveBlock3Ptr->wjShinySeen));
        gSaveBlock3Ptr->wjShinySeenMagic = WJ_SHINY_SEEN_MAGIC;
    }
}

// ---------------------------------------------------------------------------
// Nuzlocke option (owner request 2026-10-29). When the NUZLOCKE option is ON:
//  - First encounter only: in each area (region map section, so all floors of a cave are one area)
//    only the first random wild Pokemon met can be caught. Scripted/static battles (legendaries,
//    roamers, Kecleon, Voltorb...) don't count and can always be caught.
//  - Dupes clause: if every wild Pokemon in that first battle is a species you've already caught,
//    it can't be caught but the area's catch isn't used up.
//  - Shiny clause: a wild shiny can always be caught and never uses up the area (you can't run from
//    or knock out a shiny, so it must stay catchable).
//  - Fainted = gone: a fainted Pokemon stays at 0 HP; nothing heals or revives it (see HealPokemon,
//    HealBoxPokemon and the revive item checks). If the whole party faints the run is over: the
//    option switches itself OFF and the party is healed as usual, so the save never gets stuck.
//  - Forced nicknames: after a catch the naming screen opens without asking.
// ---------------------------------------------------------------------------
#define WJ_NUZLOCKE_AREA_MAGIC 0x4E
STATIC_ASSERT(MAPSEC_COUNT <= 8 * sizeof(gSaveBlock3Ptr->wjNuzlockeAreas), WallyNuzlockeAreasTooSmall);
static EWRAM_DATA u8 sWJNuzlockeCatchBlock = WJ_NUZLOCKE_CATCH_OK;

bool32 WallyJourney_NuzlockeOn(void)
{
#if IS_FRLG
    return FALSE;
#else
    return FlagGet(FLAG_WALLY_NUZLOCKE);
#endif
}

static void EnsureNuzlockeAreas(void)
{
    if (gSaveBlock3Ptr->wjNuzlockeAreaMagic != WJ_NUZLOCKE_AREA_MAGIC)
    {
        memset(gSaveBlock3Ptr->wjNuzlockeAreas, 0, sizeof(gSaveBlock3Ptr->wjNuzlockeAreas));
        gSaveBlock3Ptr->wjNuzlockeAreaMagic = WJ_NUZLOCKE_AREA_MAGIC;
    }
}

// Called by the normal random wild battle setup only (grass, water, caves, fishing, rocks, visible
// wild Pokemon), after WallyJourney_RecordWildEncounterShinies.
void WallyJourney_NuzlockeRouteEncounter(void)
{
    u32 i, count = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? 2 : 1;
    u32 mapSec = gMapHeader.regionMapSectionId;
    bool32 allDupes = TRUE;

    sWJNuzlockeCatchBlock = WJ_NUZLOCKE_CATCH_OK;
    if (!WallyJourney_NuzlockeOn() || mapSec >= MAPSEC_COUNT)
        return;
    EnsureNuzlockeAreas();
    for (i = 0; i < count; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        if (species == SPECIES_NONE)
            continue;
        if (IsMonShiny(mon))
            return; // shiny clause
        if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
            allDupes = FALSE;
    }
    if (gSaveBlock3Ptr->wjNuzlockeAreas[mapSec / 8] & (1 << (mapSec % 8)))
        sWJNuzlockeCatchBlock = WJ_NUZLOCKE_CATCH_AREA_USED;
    else if (allDupes)
        sWJNuzlockeCatchBlock = WJ_NUZLOCKE_CATCH_DUPE;
    else
        gSaveBlock3Ptr->wjNuzlockeAreas[mapSec / 8] |= 1 << (mapSec % 8); // this is the area's encounter
}

u32 WallyJourney_NuzlockeCatchBlock(void)
{
    if (!WallyJourney_NuzlockeOn() || (gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return WJ_NUZLOCKE_CATCH_OK;
    return sWJNuzlockeCatchBlock;
}

static bool32 WJ_PartyHasUsableMon(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_HP) != 0)
            return TRUE;
    }
    return FALSE;
}

// Called before the party is healed: a whole-party faint ends the Nuzlocke run.
void WallyJourney_NuzlockeCheckRunOver(void)
{
    if (WallyJourney_NuzlockeOn() && !WJ_PartyHasUsableMon())
        FlagClear(FLAG_WALLY_NUZLOCKE);
}

// TRUE when this fainted Pokemon must stay fainted.
bool32 WallyJourney_NuzlockeKeepsFainted(struct Pokemon *mon)
{
    return WallyJourney_NuzlockeOn()
        && GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
        && GetMonData(mon, MON_DATA_HP) == 0;
}

bool32 WallyJourney_NuzlockeKeepsBoxMonFainted(struct BoxPokemon *boxMon)
{
    struct Pokemon mon;
    if (!WallyJourney_NuzlockeOn() || !GetBoxMonData(boxMon, MON_DATA_SANITY_HAS_SPECIES))
        return FALSE;
    BoxMonToMon(boxMon, &mon);
    return GetMonData(&mon, MON_DATA_HP) == 0;
}

void WallyJourney_RecordWildEncounterShinies(void)
{
    u32 i, count = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? 2 : 1;
    sWJNuzlockeCatchBlock = WJ_NUZLOCKE_CATCH_OK; // only a random encounter can block catching
    EnsureShinySeen();
    for (i = 0; i < count; i++)
    {
        u16 species = GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES);
        u16 dexNum;
        if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsMonShiny(&gParties[B_TRAINER_OPPONENT_A][i]))
            continue;
        IncrementGameStat(GAME_STAT_WJ_SHINY_ENCOUNTERS);
        dexNum = SpeciesToNationalPokedexNum(species);
        if (dexNum != 0 && dexNum <= NATIONAL_DEX_COUNT)
            gSaveBlock3Ptr->wjShinySeen[dexNum / 8] |= 1 << (dexNum % 8);
    }
}

// Fills out[] with the species of every shiny met, in National Dex order; returns how many there
// are in total (which can be more than max).
u32 WallyJourney_GetShinySeenSpecies(u16 *out, u32 max)
{
    u32 dexNum, n = 0;
    EnsureShinySeen();
    for (dexNum = 1; dexNum <= NATIONAL_DEX_COUNT; dexNum++)
    {
        if (gSaveBlock3Ptr->wjShinySeen[dexNum / 8] & (1 << (dexNum % 8)))
        {
            if (n < max)
                out[n] = NationalPokedexNumToSpecies(dexNum);
            n++;
        }
    }
    return n;
}

// ---------------------------------------------------------------------------
// Catch bonus (owner request 2026-10-27): the more of a species you've caught in total (the same
// per-species count as the Hidden Ability odds above; it never resets), the better the wild ones get.
// Owner request 2026-10-29: the bonuses never stop growing (they used to top out at 31 caught).
// Every 10 more caught past 31 keeps adding at the same pace as the 21-30 -> 31 step:
//   caught    extra shiny rolls   perfect IVs   encounter weight in tables
//   0-5              0                 0              x1
//   6-10             0                 1              x1
//   11-20            3                 2              x1.25
//   21-30            7                 3              x1.5
//   31-40           11                 4              x2
//   41-50           15                 5              x2.5
//   51-60           19                 6 (all)        x3
//   ...and so on: +4 shiny rolls and +x0.5 weight per 10 more (100 caught: 35 rolls, x5).
// The count is stored per species up to 255, where it stops (x13, 99 rolls).
// The encounter boost raises that species' weight in an area's table (so rarer Pokemon still
// appear); rare visitors aren't affected.
// ---------------------------------------------------------------------------
EWRAM_DATA u16 gWJShinyRollSpecies = 0; // species being rolled for shininess (wild only), else 0

u32 WallyJourney_CatchCombo(u16 species)
{
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    EnsureCatchCounts();
    if (dexNum == 0 || dexNum > NATIONAL_DEX_COUNT)
        return 0;
    return gSaveBlock3Ptr->wjCatchCounts[dexNum];
}

// 0 below 11 caught, 1 for 11-20, 2 for 21-30, 3 for 31-40, 4 for 41-50, ...
static u32 WJ_ComboTier(u32 combo)
{
    return combo >= 11 ? (combo - 1) / 10 : 0;
}

u32 WallyJourney_CatchComboShinyRolls(u16 species)
{
    u32 tier;
    if (species == SPECIES_NONE)
        return 0;
    tier = WJ_ComboTier(WallyJourney_CatchCombo(species));
    if (tier == 0)
        return 0;
    return 4 * tier - 1; // 3, 7, 11, 15, ...
}

static u32 WJ_ComboPerfectIVs(u32 combo)
{
    u32 tier = WJ_ComboTier(combo);
    if (tier == 0)
        return combo >= 6 ? 1 : 0;
    return tier + 1 > NUM_STATS ? NUM_STATS : tier + 1;
}

// Table weight scaled for how many of this species have been caught.
u32 WallyJourney_CatchComboWeight(u16 species, u32 weight)
{
    u32 tier = WJ_ComboTier(WallyJourney_CatchCombo(species));
    u32 quarters; // multiplier in quarters: 4 = x1
    if (tier == 0)
        return weight;
    if (tier <= 2)
        quarters = 4 + tier;           // x1.25, x1.5
    else
        quarters = 8 + 2 * (tier - 3); // x2, x2.5, x3, ...
    return weight * quarters / 4;
}

// Raise a freshly made wild Pokemon's IVs for the combo: N different stats set to 31.
void WallyJourney_ApplyCatchComboIVs(struct Pokemon *mon)
{
    u32 n = WJ_ComboPerfectIVs(WallyJourney_CatchCombo(GetMonData(mon, MON_DATA_SPECIES)));
    u8 order[NUM_STATS] = {0, 1, 2, 3, 4, 5};
    u32 i, j, iv = MAX_PER_STAT_IVS;
    if (n == 0)
        return;
    for (i = NUM_STATS - 1; i > 0; i--)
    {
        u8 t;
        j = Random() % (i + 1);
        t = order[i]; order[i] = order[j]; order[j] = t;
    }
    for (i = 0; i < n; i++)
        SetMonData(mon, MON_DATA_HP_IV + order[i], &iv);
    CalculateMonStats(mon);
}

// Percent chance (0-100) a wild Pokemon of this species has its Hidden Ability:
// 5% base + 1% for every one of that species the player has caught.
u32 WallyJourney_HiddenAbilityChance(u16 species)
{
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    u32 chance = 5;
    EnsureCatchCounts();
    if (dexNum != 0 && dexNum <= NATIONAL_DEX_COUNT)
        chance += gSaveBlock3Ptr->wjCatchCounts[dexNum];
    return chance > 100 ? 100 : chance;
}

// ---------------------------------------------------------------------------
// Growing Gym Leader rematches.
// Every leader can be rematched right after the badge. Before the League their
// rematch team (TRAINER_X_2, the post-E4 team) is scaled down to the strength
// of the NEXT Gym Leader: levels match the next leader's ace, Pokemon too young
// for their evolution use the earlier form, and the team grows one member per
// badge. With 8 badges it is the full team just under the Elite Four.
// After the Hall of Fame they use TRAINER_X_3, and after the first title
// defense TRAINER_X_4 (Champion Steven's levels).
// ---------------------------------------------------------------------------
#define WJ_LEAGUE_EVE_ACE 52
static const u16 sLeaderGrowthIds[] = {
    TRAINER_ROXANNE_2, TRAINER_BRAWLY_2, TRAINER_WATTSON_2, TRAINER_FLANNERY_2,
    TRAINER_NORMAN_2, TRAINER_WINONA_2, TRAINER_TATE_AND_LIZA_2, TRAINER_JUAN_2,
};
// first-fight ace level and party size, by gym number (1-8)
static const u8 sLeaderAce[] = {0, 15, 19, 25, 32, 33, 38, 45, 46};
static const u8 sLeaderFirstCount[] = {0, 3, 3, 4, 4, 4, 5, 6, 6};
static const u8 sLeaderNames[][12] = {
    _(""), _("ROXANNE"), _("BRAWLY"), _("WATTSON"), _("FLANNERY"),
    _("NORMAN"), _("WINONA"), _("TATE&LIZA"), _("JUAN"),
};

static u32 LeaderRematchStage(void)
{
#if IS_FRLG
    return 1;
#else
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return FlagGet(FLAG_WALLY_E4_REMATCH_DONE) ? 4 : 3;
    return CountBadges() >= 8 ? 2 : 1;
#endif
}

// Special. VAR_0x8004 = gym number (1-8). VAR_RESULT = stage:
// 1 growing (before 8 badges), 2 League eve, 3 after the Hall of Fame,
// 4 after the first title defense. STR_VAR_1 = badge count, STR_VAR_2 = the
// next Gym Leader's name (the strength the team is scaled to).
void WallyJourney_SetupLeaderRematch(void)
{
    u32 badges = CountBadges();
    u32 next = badges + 1;
    if (next > 8)
        next = 8;
    gSpecialVar_Result = LeaderRematchStage();
    ConvertIntToDecimalStringN(gStringVar1, badges, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringCopy(gStringVar2, sLeaderNames[next]);
}

// Level a pre-evolution must reach to evolve into this species, or a stand-in
// for stone/friendship/trade evolutions.
static u32 EvolveLevelInto(u16 pre, u16 species)
{
    const struct Evolution *evo = GetSpeciesEvolutions(pre);
    u32 i;
    if (evo != NULL)
    {
        for (i = 0; evo[i].method != EVOLUTIONS_END; i++)
        {
            if (evo[i].targetSpecies == species && evo[i].method == EVO_LEVEL && evo[i].param > 0)
                return evo[i].param;
        }
    }
    return GetSpeciesPreEvolution(pre) != SPECIES_NONE ? 36 : 20;
}

static u16 SpeciesForLevel(u16 species, u32 level)
{
    u32 guard;
    for (guard = 0; guard < 3; guard++)
    {
        u16 pre = GetSpeciesPreEvolution(species);
        if (pre == SPECIES_NONE || level >= EvolveLevelInto(pre, species))
            break;
        species = pre;
    }
    return species;
}

static void ClearMonMoves(struct Pokemon *mon)
{
    u16 none = MOVE_NONE;
    u8 zero = 0;
    u32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        SetMonData(mon, MON_DATA_MOVE1 + i, &none);
        SetMonData(mon, MON_DATA_PP1 + i, &zero);
    }
}

// Special. VAR_0x8004 = gym number: this Leader was beaten in a post-League rematch.
void WallyJourney_RecordLeaderPostgameWin(void)
{
    if (gSpecialVar_0x8004 >= 1 && gSpecialVar_0x8004 <= 8)
        VarSet(VAR_WALLY_LEADER_POSTGAME_WINS, VarGet(VAR_WALLY_LEADER_POSTGAME_WINS) | (1 << (gSpecialVar_0x8004 - 1)));
}

// The Elite Four rematch is the last challenge: it opens once the player has
// beaten every Gym Leader's post-League team, the final rival battle at Birch's
// lab and Steven in Meteor Falls.
bool8 WallyJourney_IsEliteFourRematchUnlocked(void)
{
#if IS_FRLG
    return FALSE;
#else
    return FlagGet(FLAG_SYS_GAME_CLEAR)
        && (VarGet(VAR_WALLY_LEADER_POSTGAME_WINS) & 0xFF) == 0xFF
        && FlagGet(FLAG_WALLY_RIVAL_FINAL_DONE)
        && FlagGet(FLAG_DEFEATED_METEOR_FALLS_STEVEN);
#endif
}

bool32 WallyJourney_IsLeaderRematchTrainer(u16 trainerId)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sLeaderGrowthIds); i++)
    {
        // each leader's _2, _3 and _4 slots are consecutive
        if (trainerId >= sLeaderGrowthIds[i] && trainerId <= sLeaderGrowthIds[i] + 2)
            return TRUE;
    }
    return FALSE;
}

// May/Brendan's final team is levels 67-72 for the postgame battle on the way to the
// Elite Four rematch. As a title challenger (5th defense on) the rest of the pool is at
// 75-78, so there the same team is raised so its ace is level 78.
#define WJ_TITLE_POOL_ACE 78
static void WallyJourney_ScaleTitleRivalParty(struct Pokemon *party, u16 trainerId)
{
    static const u16 sRivalFinals[] = {
        TRAINER_MAY_FINAL_TREECKO, TRAINER_MAY_FINAL_TORCHIC, TRAINER_MAY_FINAL_MUDKIP,
        TRAINER_BRENDAN_FINAL_TREECKO, TRAINER_BRENDAN_FINAL_TORCHIC, TRAINER_BRENDAN_FINAL_MUDKIP,
    };
    u32 i, n;
    s32 shift;
    for (i = 0; i < ARRAY_COUNT(sRivalFinals); i++)
        if (sRivalFinals[i] == trainerId)
            break;
    if (i == ARRAY_COUNT(sRivalFinals)
     || gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM))
        return;
    for (n = 0; n < PARTY_SIZE && GetMonData(&party[n], MON_DATA_SPECIES) != SPECIES_NONE; n++)
        ;
    if (n == 0)
        return;
    shift = WJ_TITLE_POOL_ACE - (s32)GetMonData(&party[n - 1], MON_DATA_LEVEL);
    if (shift <= 0)
        return;
    for (i = 0; i < n; i++)
    {
        struct Pokemon *mon = &party[i];
        s32 level = (s32)GetMonData(mon, MON_DATA_LEVEL) + shift;
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp;
        if (level > MAX_LEVEL)
            level = MAX_LEVEL;
        exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        SetMonData(mon, MON_DATA_LEVEL, &level);
        CalculateMonStats(mon);
    }
}

// Vito as a title-defense challenger (owner request 2026-10-29): in the Champion's Room his team
// scales to the player. His ace is raised to the level of the player's highest-level party Pokemon
// (never below his Victory Road level, 48) and the rest of his team keeps the same gaps below it.
// His Victory Road battle is unchanged.
static void WallyJourney_ScaleVitoParty(struct Pokemon *party, u16 trainerId)
{
    u32 i, n, playerTop = 0, vitoTop = 0;
    s32 shift;
    if (trainerId != TRAINER_VITO
     || gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM))
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_LEVEL) > playerTop)
            playerTop = GetMonData(mon, MON_DATA_LEVEL);
    }
    for (n = 0; n < PARTY_SIZE && GetMonData(&party[n], MON_DATA_SPECIES) != SPECIES_NONE; n++)
    {
        if (GetMonData(&party[n], MON_DATA_LEVEL) > vitoTop)
            vitoTop = GetMonData(&party[n], MON_DATA_LEVEL);
    }
    shift = (s32)playerTop - (s32)vitoTop;
    if (n == 0 || shift <= 0)
        return;
    for (i = 0; i < n; i++)
    {
        struct Pokemon *mon = &party[i];
        s32 level = (s32)GetMonData(mon, MON_DATA_LEVEL) + shift;
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp;
        if (level > MAX_LEVEL)
            level = MAX_LEVEL;
        exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        SetMonData(mon, MON_DATA_LEVEL, &level);
        CalculateMonStats(mon);
    }
}

void WallyJourney_ScaleLeaderParty(struct Pokemon *party, u16 trainerId)
{
#if !IS_FRLG
    u32 gym, i, n, count, badges, target, aceLevel, stage;
    WallyJourney_ScaleTitleRivalParty(party, trainerId);
    WallyJourney_ScaleVitoParty(party, trainerId);
    for (gym = 0; gym < ARRAY_COUNT(sLeaderGrowthIds); gym++)
        if (sLeaderGrowthIds[gym] == trainerId)
            break;
    if (gym == ARRAY_COUNT(sLeaderGrowthIds))
        return;
    gym++;
    stage = LeaderRematchStage();
    if (stage > 2)
        return;
    badges = CountBadges();
    for (n = 0; n < PARTY_SIZE && GetMonData(&party[n], MON_DATA_SPECIES) != SPECIES_NONE; n++)
        ;
    if (n == 0)
        return;
    if (stage == 2)
    {
        target = WJ_LEAGUE_EVE_ACE;
        count = n;
    }
    else
    {
        target = sLeaderAce[badges + 1 > 8 ? 8 : badges + 1];
        count = sLeaderFirstCount[gym] + (badges > gym ? badges - gym : 0);
        if (count > n)
            count = n;
    }
    aceLevel = GetMonData(&party[n - 1], MON_DATA_LEVEL);
    // keep the first count-1 members and the ace
    if (count < n)
    {
        party[count - 1] = party[n - 1];
        for (i = count; i < n; i++)
            ZeroMonData(&party[i]);
    }
    for (i = 0; i < count; i++)
    {
        struct Pokemon *mon = &party[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);
        s32 level = (s32)target - ((s32)aceLevel - (s32)GetMonData(mon, MON_DATA_LEVEL));
        u16 newSpecies;
        u32 exp;
        if (level < 2)
            level = 2;
        newSpecies = SpeciesForLevel(species, level);
        if (newSpecies != species)
        {
            SetMonData(mon, MON_DATA_SPECIES, &newSpecies);
            SetMonData(mon, MON_DATA_NICKNAME, GetSpeciesName(newSpecies));
        }
        // No Mega Evolution before the player has the Mega Bracelet (5 badges)
        // or when the Pokemon isn't the stone's species yet.
        if (GetItemHoldEffect(item) == HOLD_EFFECT_MEGA_STONE && (badges < 5 || newSpecies != species))
        {
            u16 none = ITEM_NONE;
            SetMonData(mon, MON_DATA_HELD_ITEM, &none);
        }
        exp = gExperienceTables[gSpeciesInfo[newSpecies].growthRate][level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        SetMonData(mon, MON_DATA_LEVEL, &level);
        // Before the League eve the hand-picked late-game moves are too strong:
        // use what the Pokemon knows by level-up at this level.
        if (stage == 1 || newSpecies != species)
        {
            ClearMonMoves(mon);
            GiveMonInitialMoveset(mon);
        }
        CalculateMonStats(mon);
    }
#endif
}

// Called when the Options EXP. MODE is saved, and on map entry for older saves:
// swaps Devon's Exp. Share and the EXP. ALL key item to match the mode.
void WallyJourney_SyncExpItem(void)
{
#if !IS_FRLG
    if (!FlagGet(FLAG_RECEIVED_EXP_SHARE))
        return;
    if (FlagGet(FLAG_WALLY_EXP_ALL_MODE))
    {
        if (!CheckBagHasItem(ITEM_EXP_ALL, 1) && TakeExpShare())
        {
            AddBagItem(ITEM_EXP_ALL, 1);
            FlagSet(FLAG_WALLY_EXP_ALL_ON);
        }
    }
    else if (CheckBagHasItem(ITEM_EXP_ALL, 1))
    {
        RemoveBagItem(ITEM_EXP_ALL, 1);
        AddBagItem(ITEM_EXP_SHARE, 1);
    }
#endif
}

bool32 WallyJourney_ZMovesEnabled(void)
{
#if IS_FRLG
    return TRUE;
#else
    return !FlagGet(FLAG_WALLY_Z_MOVES_OFF);
#endif
}

// Script special (after setwildbattle): the Sky Pillar Rayquaza knows Dragon
// Ascent, which is what lets Rayquaza Mega Evolve without a Mega Stone.
void WallyJourney_GiveWildRayquazaDragonAscent(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_RAYQUAZA)
        SetMonMoveSlot(mon, MOVE_DRAGON_ASCENT, 0);
}

// ---------------------------------------------------------------- seasons
// Owner design (2026-10-17): each season lasts 48 real hours, cycling
// spring -> summer -> autumn -> winter every 8 days. Driven by the real-time
// clock, so time keeps passing while the game is off and the season is the
// same on every map and after every reload.
#define WALLY_HOURS_PER_SEASON 48
// Owner request (2026-10-27): every save on a PC set to the same clock is in the same season and
// sees the same weather, so friends playing together match. Seasons and weather therefore run on
// the real date and hour from the clock itself (hours since 1 Jan 2000), not on the save's own
// in-game clock (which starts when each save is made). The offset lines the cycle up so autumn
// starts at midnight on 27 Oct 2026: autumn 27-28 Oct, winter 29-30, spring 31 Oct-1 Nov,
// summer 2-3 Nov, autumn again 4-5 Nov, and so on every 8 days.
#define WALLY_SEASON_PHASE_HOURS 168
static s32 WJ_RealClockHour(void)
{
    struct SiiRtcInfo rtc;
    RtcGetInfo(&rtc);
    return (s32)RtcGetDayCount(&rtc) * 24 + (s32)ConvertBcdToBinary(rtc.hour);
}

enum WallySeason WallyJourney_GetSeason(void)
{
    s32 hours;
    if (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK)
        return WALLY_SEASON_SUMMER; // no or broken clock: plain Hoenn summer, no tint surprises
    hours = WJ_RealClockHour() + WALLY_SEASON_PHASE_HOURS;
    return (hours / WALLY_HOURS_PER_SEASON) % 4;
}

// Daytime tint per season, layered into the engine's day/night blend.
// Colors are per-channel multipliers (R low byte), 255 = unchanged.
#define TINT(r, g, b) ((r) | ((g) << 8) | ((b) << 16))
struct BlendSettings WallyJourney_GetSeasonDayBlend(void)
{
    struct BlendSettings blend = {.coeff = 0, .blendColor = 0, .isTint = FALSE};
    switch (WallyJourney_GetSeason())
    {
    case WALLY_SEASON_SPRING: blend.isTint = TRUE; blend.blendColor = TINT(250, 238, 246); break; // soft blossom pink
    case WALLY_SEASON_SUMMER: break;                                                              // classic bright Hoenn
    case WALLY_SEASON_AUTUMN: blend.isTint = TRUE; blend.blendColor = TINT(255, 236, 212); break; // soft amber light (the leaves carry the colour)
    case WALLY_SEASON_WINTER: blend.isTint = TRUE; blend.blendColor = TINT(214, 226, 255); break; // cool, pale light
    }
    return blend;
}
#undef TINT

// Script special: VAR_RESULT = current season (0 spring, 1 summer, 2 autumn,
// 3 winter); summer when the clock is broken, so NPCs fall back to their
// normal line.
void WallyJourney_GetSeasonForScript(void)
{
    if (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK)
        gSpecialVar_Result = WALLY_SEASON_SUMMER;
    else
        gSpecialVar_Result = WallyJourney_GetSeason();
}

// ------------------------------------------------------ living weather
// Owner design (2026-10-17): each part of Hoenn has its own weather, set by
// region, season and hour. Weather fronts roll across Hoenn from west to east,
// so neighbouring areas see the same front a few hours apart, and every front
// builds up (clouds, overcast), peaks (rain or storm) and clears again.
// Everything is computed from the real-time clock, so weather is the same on
// every reload and never needs saving. Story weather is never touched.

enum WallyRegion
{
    WREGION_NONE,
    WREGION_WEST_COAST,  // Petalburg, Rustboro, Dewford, Slateport, Routes 104-109
    WREGION_INLAND,      // Littleroot to Mauville, Verdanturf, Routes 101-103, 110, 116-118
    WREGION_DESERT,      // Route 111
    WREGION_HIGHLAND,    // Fallarbor, Routes 112, 114, 115
    WREGION_RAINFOREST,  // Fortree, Routes 119-120
    WREGION_EAST_COAST,  // Lilycove, Mossdeep, Routes 121-125
    WREGION_OCEAN,       // Sootopolis, Pacifidlog, Ever Grande, Routes 126-134
};

// Hours a front takes to reach each region after leaving the west coast.
static const u8 sRegionFrontDelay[] =
{
    [WREGION_WEST_COAST] = 0, [WREGION_INLAND] = 2, [WREGION_DESERT] = 3, [WREGION_HIGHLAND] = 3,
    [WREGION_RAINFOREST] = 5, [WREGION_EAST_COAST] = 6, [WREGION_OCEAN] = 8,
};

static enum WallyRegion GetRegionForMapSec(u32 mapSec)
{
    switch (mapSec)
    {
    case MAPSEC_PETALBURG_CITY: case MAPSEC_RUSTBORO_CITY: case MAPSEC_DEWFORD_TOWN: case MAPSEC_SLATEPORT_CITY:
    case MAPSEC_ROUTE_104: case MAPSEC_ROUTE_105: case MAPSEC_ROUTE_106: case MAPSEC_ROUTE_107:
    case MAPSEC_ROUTE_108: case MAPSEC_ROUTE_109:
        return WREGION_WEST_COAST;
    case MAPSEC_LITTLEROOT_TOWN: case MAPSEC_OLDALE_TOWN: case MAPSEC_VERDANTURF_TOWN: case MAPSEC_MAUVILLE_CITY:
    case MAPSEC_LAVARIDGE_TOWN:
    case MAPSEC_ROUTE_101: case MAPSEC_ROUTE_102: case MAPSEC_ROUTE_103: case MAPSEC_ROUTE_110:
    case MAPSEC_ROUTE_116: case MAPSEC_ROUTE_117: case MAPSEC_ROUTE_118:
        return WREGION_INLAND;
    case MAPSEC_ROUTE_111:
        return WREGION_DESERT;
    case MAPSEC_FALLARBOR_TOWN: case MAPSEC_ROUTE_112: case MAPSEC_ROUTE_114: case MAPSEC_ROUTE_115:
        return WREGION_HIGHLAND;
    case MAPSEC_FORTREE_CITY: case MAPSEC_ROUTE_119: case MAPSEC_ROUTE_120:
        return WREGION_RAINFOREST;
    case MAPSEC_LILYCOVE_CITY: case MAPSEC_MOSSDEEP_CITY: case MAPSEC_ROUTE_121: case MAPSEC_ROUTE_122:
    case MAPSEC_ROUTE_123: case MAPSEC_ROUTE_124: case MAPSEC_ROUTE_125:
        return WREGION_EAST_COAST;
    case MAPSEC_SOOTOPOLIS_CITY: case MAPSEC_PACIFIDLOG_TOWN: case MAPSEC_EVER_GRANDE_CITY:
    case MAPSEC_ROUTE_126: case MAPSEC_ROUTE_127: case MAPSEC_ROUTE_128: case MAPSEC_ROUTE_129:
    case MAPSEC_ROUTE_130: case MAPSEC_ROUTE_131: case MAPSEC_ROUTE_132: case MAPSEC_ROUTE_133:
    case MAPSEC_ROUTE_134:
        return WREGION_OCEAN;
    }
    return WREGION_NONE;
}

// Owner request 2026-10-29: winter is snowy on every route and town (it used to be only the
// highland routes 112/114/115 and Mt. Pyre, so most of Hoenn barely looked different from summer).
// Every outdoor map gets snow-covered ground and frosted trees, and winter rain falls as snow.
// The desert still stays dry (fronts pass over it), so Route 111 is snowy ground without snowfall.
static bool32 IsSnowCountry(void)
{
    return TRUE;
}

static u32 WeatherHash(u32 a)
{
    a ^= a >> 16; a *= 0x7FEB352Du;
    a ^= a >> 15; a *= 0x846CA68Bu;
    a ^= a >> 16;
    return a;
}

static enum WallySeason SeasonAtHour(s32 hour)
{
    if (hour < 0)
        hour = 0;
    return ((hour + WALLY_SEASON_PHASE_HOURS) / WALLY_HOURS_PER_SEASON) % 4;
}

enum { FRONT_SHOWERS, FRONT_RAIN, FRONT_STORM };

// At most one front leaves the west coast per real day.
static bool32 GetFront(s32 day, s32 *start, u32 *length, u32 *strength)
{
    static const u8 sFrontChance[4] = {55, 45, 55, 40}; // spring, summer, autumn, winter
    u32 h = WeatherHash((u32)day * 2654435761u ^ 0x5EA50017u);
    enum WallySeason season = SeasonAtHour(day * 24);
    u32 roll = (h >> 20) % 100;
    if (h % 100 >= sFrontChance[season])
        return FALSE;
    *start = day * 24 + (h >> 8) % 24;
    switch (season)
    {
    case WALLY_SEASON_SPRING: *length = 6 + (h >> 13) % 6;  *strength = roll < 40 ? FRONT_SHOWERS : roll < 90 ? FRONT_RAIN : FRONT_STORM; break;
    case WALLY_SEASON_SUMMER: *length = 4 + (h >> 13) % 4;  *strength = roll < 50 ? FRONT_STORM : FRONT_SHOWERS; break;
    case WALLY_SEASON_AUTUMN: *length = 8 + (h >> 13) % 8;  *strength = roll < 10 ? FRONT_SHOWERS : roll < 90 ? FRONT_RAIN : FRONT_STORM; break;
    default:                  *length = 8 + (h >> 13) % 6;  *strength = roll < 10 ? FRONT_SHOWERS : FRONT_RAIN; break;
    }
    return TRUE;
}

// Weather of a region at an absolute real hour, before map-specific rules.
static u32 RegionWeatherAt(enum WallyRegion region, s32 hour, u32 fairWeather, bool32 snowCountry)
{
    s32 local = hour - sRegionFrontDelay[region];
    s32 day = local >= 0 ? local / 24 : -1;
    s32 d, start;
    u32 length, strength, w = fairWeather;
    enum WallySeason season = SeasonAtHour(hour);
    u32 hourOfDay = (u32)hour % 24;
    u32 daily = WeatherHash((u32)(hour / 24) * 40503u + region * 977u);

    for (d = day - 1; d <= day; d++)
    {
        s32 phase;
        if (d < 0 || !GetFront(d, &start, &length, &strength))
            continue;
        phase = local - start;
        if (phase < 0 || phase >= (s32)length)
            continue;
        // No WEATHER_SUNNY_CLOUDS here: its cloud sprites show through any
        // see-through tile (ledges, not just water) on most maps (owner
        // playtest 2026-10-17). Overcast skies use SHADE instead.
        if (phase == 0 || phase == (s32)length - 1)
            w = (strength == FRONT_SHOWERS) ? fairWeather : WEATHER_SHADE; // skies darken / start to clear
        else if (phase == 1 && strength != FRONT_SHOWERS)
            w = WEATHER_SHADE;                              // overcast
        else if (strength == FRONT_STORM && phase >= 3 && phase <= (s32)length - 3)
            w = WEATHER_RAIN_THUNDERSTORM;                  // the peak of the storm
        else
            w = WEATHER_RAIN;
        break;
    }

    if (w == fairWeather)
    {
        // No front: local character.
        switch (region)
        {
        case WREGION_RAINFOREST:
            if (hourOfDay >= 14 && hourOfDay <= 17 && daily % 100 < 40)
                w = WEATHER_RAIN;                           // daily rainforest downpour
            else if (hourOfDay >= 5 && hourOfDay <= 8 && (daily >> 8) % 100 < 35)
                w = WEATHER_FOG_HORIZONTAL;                 // morning mist in the canopy
            break;
        case WREGION_OCEAN:
            if (season == WALLY_SEASON_SUMMER && hourOfDay >= 13 && hourOfDay <= 15 && daily % 100 < 30)
                w = WEATHER_RAIN;                           // tropical afternoon shower
            else if (hourOfDay >= 5 && hourOfDay <= 7 && (daily >> 8) % 100 < 20)
                w = WEATHER_FOG_HORIZONTAL;                 // sea mist at dawn
            break;
        case WREGION_HIGHLAND:
            if (hourOfDay >= 5 && hourOfDay <= 8 && (daily >> 8) % 100 < 25)
                w = WEATHER_FOG_HORIZONTAL;                 // mountain mist
            break;
        case WREGION_WEST_COAST:
        case WREGION_EAST_COAST:
            if (hourOfDay >= 5 && hourOfDay <= 6 && (daily >> 8) % 100 < 12)
                w = WEATHER_FOG_HORIZONTAL;                 // coastal fog
            break;
        default:
            break;
        }
        if (w == fairWeather && season == WALLY_SEASON_AUTUMN && (daily >> 16) % 100 < 25)
            w = WEATHER_SHADE;                              // grey autumn day
    }

    if (region == WREGION_DESERT && w != fairWeather)
        w = fairWeather;                                    // fronts pass over the desert dry
    if (season == WALLY_SEASON_WINTER)
    {
        if (snowCountry && (w == WEATHER_RAIN || w == WEATHER_RAIN_THUNDERSTORM || w == WEATHER_FOG_HORIZONTAL))
            w = WEATHER_SNOW;                               // mountain snow
        else if (w == WEATHER_RAIN_THUNDERSTORM)
            w = WEATHER_RAIN;                               // cold rain, no thunder
    }
    return w;
}

// Weather this code chose for the current map, stored +1 so 0 (the EWRAM
// default) means the map's weather is not ours.
static EWRAM_DATA u8 sAutoWeatherPlus1 = 0;
static EWRAM_DATA u8 sAutoFairWeather = 0;

static s32 CurrentRealHour(void)
{
    return WJ_RealClockHour();
}

// The weather this system wants on the current map right now, or FALSE when
// the map keeps its own weather (indoors, ash, story weather, broken clock...).
static bool32 GetRegionalWeatherForCurrentMap(u32 *out)
{
    u32 header = gMapHeader.weather;
    enum MapType type = gMapHeader.mapType;
    enum WallyRegion region = GetRegionForMapSec(gMapHeader.regionMapSectionId);

    if (header != WEATHER_SUNNY && header != WEATHER_SUNNY_CLOUDS
     && header != WEATHER_ROUTE119_CYCLE && header != WEATHER_ROUTE123_CYCLE)
        return FALSE;
    if (FlagGet(FLAG_SYS_WEATHER_CTRL))
        return FALSE;
    if (type != MAP_TYPE_ROUTE && type != MAP_TYPE_TOWN && type != MAP_TYPE_CITY && type != MAP_TYPE_OCEAN_ROUTE)
        return FALSE;
    if (region == WREGION_NONE || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return FALSE;
    sAutoFairWeather = (header == WEATHER_SUNNY_CLOUDS) ? WEATHER_SUNNY_CLOUDS : WEATHER_SUNNY;
    *out = RegionWeatherAt(region, CurrentRealHour(), sAutoFairWeather, IsSnowCountry());
    return TRUE;
}

// Called when a map's weather is loaded from its header.
static EWRAM_DATA bool8 sSeasonCheckPending = FALSE;

u32 WallyJourney_SeasonalWeather(u32 weather)
{
    u32 w;
    sSeasonCheckPending = TRUE;
    sAutoWeatherPlus1 = 0;
    if (!GetRegionalWeatherForCurrentMap(&w))
        return weather;
    sAutoWeatherPlus1 = w + 1;
    return w;
}

// Coord-event weather tiles (Routes 111, 119, 120, 123) that only switch
// between ordinary sunny/cloudy/rain/cycle weather defer to the regional
// weather. Desert sandstorm, ash, fog and shade tiles keep working.
bool32 WallyJourney_TryRegionalCoordWeather(u32 coordWeather)
{
    u32 w;
    if (coordWeather != COORD_EVENT_WEATHER_SUNNY && coordWeather != COORD_EVENT_WEATHER_SUNNY_CLOUDS
     && coordWeather != COORD_EVENT_WEATHER_RAIN && coordWeather != COORD_EVENT_WEATHER_ROUTE119_CYCLE
     && coordWeather != COORD_EVENT_WEATHER_ROUTE123_CYCLE)
        return FALSE;
    if (!GetRegionalWeatherForCurrentMap(&w))
        return FALSE;
    sAutoWeatherPlus1 = w + 1;
    SetSavedWeather(w);
    SetNextWeather(w);
    return TRUE;
}

// Called about once a minute in the overworld: lets weather change while the
// player stands on a map, fading in and out through the normal weather engine.
void WallyJourney_UpdateLiveWeather(void)
{
    u32 w;
    sSeasonCheckPending = TRUE;
    if (sAutoWeatherPlus1 == 0 || gMain.callback2 != CB2_Overworld)
        return;
    if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return;
    if (GetSavedWeather() != sAutoWeatherPlus1 - 1 || FlagGet(FLAG_SYS_WEATHER_CTRL))
    {
        sAutoWeatherPlus1 = 0; // a script or coord event took over this map's weather
        return;
    }
    w = RegionWeatherAt(GetRegionForMapSec(gMapHeader.regionMapSectionId), CurrentRealHour(), sAutoFairWeather, IsSnowCountry());
    if (w == sAutoWeatherPlus1 - 1u)
        return;
    sAutoWeatherPlus1 = w + 1;
    SetSavedWeather(w);
    SetNextWeather(w);
}

// "Autumn has arrived in HOENN." Shown once per season change, the first
// time the player is outdoors and free to move. A brand-new save starts
// quietly.
extern const u8 WallyJourney_EventScript_SeasonSpring[];
extern const u8 WallyJourney_EventScript_SeasonSummer[];
extern const u8 WallyJourney_EventScript_SeasonAutumn[];
extern const u8 WallyJourney_EventScript_SeasonWinter[];

void WallyJourney_TrySeasonNotice(void)
{
    static const u8 *const sScripts[] =
    {
        WallyJourney_EventScript_SeasonSpring, WallyJourney_EventScript_SeasonSummer,
        WallyJourney_EventScript_SeasonAutumn, WallyJourney_EventScript_SeasonWinter,
    };
    u32 season, seen;
    if (!sSeasonCheckPending)
        return;
    if (gMain.callback2 != CB2_Overworld || gPaletteFade.active
     || ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return;
    sSeasonCheckPending = FALSE;
    if (!MapHasNaturalLight(gMapHeader.mapType) || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return;
    season = WallyJourney_GetSeason();
    seen = VarGet(VAR_WALLY_SEASON_SEEN);
    if (seen == season + 1)
        return;
    VarSet(VAR_WALLY_SEASON_SEEN, season + 1);
    if (seen == 0 || FlagGet(FLAG_SYS_WEATHER_CTRL))
        return;
    ScriptContext_SetupScript(sScripts[season]);
}

// PokéNav forecast: the same schedule the routes use.
static const u8 sWeatherName_Sunny[] = _("Sunny");
static const u8 sWeatherName_Cloudy[] = _("Cloudy");
static const u8 sWeatherName_Overcast[] = _("Overcast");
static const u8 sWeatherName_Rain[] = _("Rain");
static const u8 sWeatherName_Storm[] = _("Storm");
static const u8 sWeatherName_Snow[] = _("Snow");
static const u8 sWeatherName_Mist[] = _("Mist");

static const u8 *ForecastWeatherName(u32 weather)
{
    switch (weather)
    {
    case WEATHER_SUNNY_CLOUDS:      return sWeatherName_Cloudy;
    case WEATHER_SHADE:             return sWeatherName_Overcast;
    case WEATHER_RAIN:              return sWeatherName_Rain;
    case WEATHER_RAIN_THUNDERSTORM: return sWeatherName_Storm;
    case WEATHER_SNOW:              return sWeatherName_Snow;
    case WEATHER_FOG_HORIZONTAL:    return sWeatherName_Mist;
    default:                        return sWeatherName_Sunny;
    }
}

static bool32 IsWetWeather(u32 weather)
{
    return weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_SNOW;
}

static const u8 sForecastNow[] = _("Now: ");
static const u8 sForecastIn[] = _(" in ");
static const u8 sForecastHours[] = _("h");
static const u8 sForecastClearing[] = _("Clearing");
static const u8 sForecastSteady[] = _("No change soon");

// Two short lines for the PokéNav map, e.g. "Now: Rain" / "Clearing in 3h".
// FALSE for places that keep their own weather.
bool32 WallyJourney_GetForecast(u32 mapSec, u8 *now, u8 *later)
{
    enum WallyRegion region = GetRegionForMapSec(mapSec);
    bool32 snow = (mapSec == MAPSEC_ROUTE_112 || mapSec == MAPSEC_ROUTE_114 || mapSec == MAPSEC_ROUTE_115);
    s32 hour, h;
    u32 cur, next = 0;

    if (region == WREGION_NONE || FlagGet(FLAG_SYS_WEATHER_CTRL) || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return FALSE;
    hour = CurrentRealHour();
    cur = RegionWeatherAt(region, hour, WEATHER_SUNNY, snow);
    for (h = 1; h <= 12; h++)
    {
        next = RegionWeatherAt(region, hour + h, WEATHER_SUNNY, snow);
        if (IsWetWeather(next) != IsWetWeather(cur) || (IsWetWeather(cur) && next != cur))
            break;
    }
    now = StringCopy(now, sForecastNow);
    StringCopy(now, ForecastWeatherName(cur));
    if (h > 12)
    {
        StringCopy(later, sForecastSteady);
        return TRUE;
    }
    later = StringCopy(later, IsWetWeather(next) ? ForecastWeatherName(next) : sForecastClearing);
    later = StringCopy(later, sForecastIn);
    later = ConvertIntToDecimalStringN(later, h, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(later, sForecastHours);
    return TRUE;
}

// ------------------------------------------------------ seasonal foliage
// Tileset palettes of outdoor maps are recoloured per season when they load:
// fresh yellow-green spring with pink blossoms on some tree types, untouched summer, red/orange/gold autumn (each
// palette gets its own colour so trees don't all turn the same), dull winter
// green on the coast and frosted pale foliage and snow-covered ground on the
// snowy highland routes.
// Only greens are touched: water, sand, roofs in other colours, people and
// Pokémon stay as they are. Layout, collision and access never change.
enum { FOLIAGE_NONE, FOLIAGE_SPRING, FOLIAGE_AUTUMN, FOLIAGE_WINTER, FOLIAGE_FROST };

static EWRAM_DATA u8 sPrimaryFoliagePlus1 = 0;

static u32 CurrentFoliageMode(void)
{
    if (!MapHasNaturalLight(gMapHeader.mapType) || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return FOLIAGE_NONE;
    switch (WallyJourney_GetSeason())
    {
    case WALLY_SEASON_SPRING: return FOLIAGE_SPRING;
    case WALLY_SEASON_AUTUMN: return FOLIAGE_AUTUMN;
    case WALLY_SEASON_WINTER: return IsSnowCountry() ? FOLIAGE_FROST : FOLIAGE_WINTER;
    default:                  return FOLIAGE_NONE;
    }
}

static u16 SeasonalColor(u16 color, u32 mode, u32 slot)
{
    static const s16 sAutumnHues[] = {40, 14, 26, 34}; // gold, red, orange, amber
    s32 r = color & 31, g = (color >> 5) & 31, b = (color >> 10) & 31;
    s32 max = max(r, max(g, b)), min = min(r, min(g, b)), d = max - min;
    s32 h, s, v, i, f, p, q, tt;

    if (max == 0 || d * 100 / max < 18)
        return color; // greys, black, white: leave alone
    if (max == r)      h = 60 * (g - b) / d;
    else if (max == g) h = 60 * (b - r) / d + 120;
    else               h = 60 * (r - g) / d + 240;
    if (h < 0)
        h += 360;
    s = d * 255 / max;
    v = max;
    if (mode == FOLIAGE_FROST && h >= 10 && h < 65 && v >= 20)
    {
        // Snow cover: light earth, paths and sand turn white on the snowy
        // highland routes. Dark cliff faces keep their colour.
        h = 210;
        s = s * 15 / 100;
        v += (31 - v) * 70 / 100;
    }
    else if (h < 65 || h > 170)
    {
        return color; // not foliage
    }
    else
    {
        switch (mode)
        {
        case FOLIAGE_SPRING:
            if (slot % 4 == 1)
            {
                // Blossom trees: some tree types flower pink in spring.
                h = 335 + (h - 110) / 8;
                s = s * 55 / 100;
                v = min(31, v + 3);
            }
            else
            {
                h -= 14;
                s = min(255, s * 120 / 100);
                v = min(31, v + 2);
            }
            break;
        case FOLIAGE_AUTUMN:
            h = sAutumnHues[slot % ARRAY_COUNT(sAutumnHues)] + (v - 16) / 2 + (h - 110) / 8;
            s = min(255, s * 115 / 100);
            break;
        case FOLIAGE_WINTER:
            s = s * 60 / 100;
            v = v * 92 / 100;
            break;
        case FOLIAGE_FROST:
            h += (200 - h) / 2;
            s = s * 20 / 100;
            v += (31 - v) * 60 / 100;
            break;
        default:
            return color;
        }
    }
    if (h < 0)
        h += 360;
    h %= 360;

    i = h / 60;
    f = (h % 60) * 255 / 60;
    p = v * (255 - s) / 255;
    q = v * (255 - s * f / 255) / 255;
    tt = v * (255 - s * (255 - f) / 255) / 255;
    switch (i)
    {
    case 0:  r = v;  g = tt; b = p;  break;
    case 1:  r = q;  g = v;  b = p;  break;
    case 2:  r = p;  g = v;  b = tt; break;
    case 3:  r = p;  g = q;  b = v;  break;
    case 4:  r = tt; g = p;  b = v;  break;
    default: r = v;  g = p;  b = q;  break;
    }
    return RGB(r, g, b);
}

// Recolour BG palettes [palStart, palStart + palCount) just loaded from a tileset.
void WallyJourney_ApplySeasonalFoliage(u32 palStart, u32 palCount, bool32 alsoFaded, bool32 isPrimary)
{
    u32 mode = CurrentFoliageMode();
    u32 slot, c;
    if (isPrimary)
        sPrimaryFoliagePlus1 = mode + 1;
    if (mode == FOLIAGE_NONE)
        return;
    for (slot = palStart; slot < palStart + palCount; slot++)
    {
        for (c = 1; c < 16; c++)
        {
            u16 *col = &gPlttBufferUnfaded[slot * 16 + c];
            *col = SeasonalColor(*col, mode, slot);
            if (alsoFaded)
                gPlttBufferFaded[slot * 16 + c] = *col;
        }
    }
}

// TRUE when the primary tileset palette was coloured for a different
// season/place than the map just entered through a connection.
bool32 WallyJourney_PrimaryFoliageStale(void)
{
    return sPrimaryFoliagePlus1 != CurrentFoliageMode() + 1;
}

// ------------------------------------------------- falling leaves / petals
// Autumn leaves and spring blossom petals drift across the screen on outdoor
// maps in fair weather (not on the open sea or in the desert). A handful of
// small sprites, recreated automatically after battles and map changes.
#define FALLING_LEAF_COUNT   6
#define TAG_WALLY_FALLING_LEAF 0x5A11

static const u32 sFallingLeafGfx[] = INCGFX_U32("graphics/wally/falling_leaf.png", ".4bpp");
static const u16 sFallingLeafPal_Autumn[16] =
{
    RGB(0, 0, 0), RGB(15, 5, 2), RGB(26, 11, 3), RGB(31, 21, 6),
};
static const u16 sFallingLeafPal_Spring[16] =
{
    RGB(0, 0, 0), RGB(24, 12, 17), RGB(30, 19, 24), RGB(31, 27, 29),
};

static const struct OamData sFallingLeafOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};
static const struct SpriteFrameImage sFallingLeafImages[] =
{
    {(const u8 *)sFallingLeafGfx, 0x20},
    {(const u8 *)sFallingLeafGfx + 0x20, 0x20},
};
static const union AnimCmd sFallingLeafAnim[] =
{
    ANIMCMD_FRAME(0, 14),
    ANIMCMD_FRAME(1, 10),
    ANIMCMD_FRAME(0, 14, .hFlip = TRUE),
    ANIMCMD_FRAME(1, 10, .vFlip = TRUE),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd *const sFallingLeafAnims[] = {sFallingLeafAnim};

static void UpdateFallingLeaf(struct Sprite *sprite);
static const struct SpriteTemplate sFallingLeafTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_WALLY_FALLING_LEAF,
    .oam = &sFallingLeafOam,
    .anims = sFallingLeafAnims,
    .images = sFallingLeafImages,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateFallingLeaf,
};

#define tLeafPosY   data[0]
#define tLeafSpeedY data[1]
#define tLeafWave   data[2]
#define tLeafDrift  data[3]

static void ResetFallingLeaf(struct Sprite *sprite, bool32 anywhere)
{
    u32 r = Random();
    sprite->x = (r % 240) - (gSpriteCoordOffsetX + sprite->centerToCornerVecX);
    sprite->y = (anywhere ? (r >> 8) % 160 : 0) - 8 - (gSpriteCoordOffsetY + sprite->centerToCornerVecY);
    sprite->tLeafPosY = sprite->y * 128;
    sprite->tLeafSpeedY = 40 + (r >> 16) % 40;
    sprite->tLeafWave = (r >> 20) & 0xFF;
    sprite->tLeafDrift = 1 + ((r >> 28) & 1);
}

static void UpdateFallingLeaf(struct Sprite *sprite)
{
    s16 x, y;
    sprite->tLeafPosY += sprite->tLeafSpeedY;
    sprite->y = sprite->tLeafPosY >> 7;
    sprite->tLeafWave = (sprite->tLeafWave + sprite->tLeafDrift) & 0xFF;
    sprite->x2 = gSineTable[sprite->tLeafWave] / 32;
    if ((gMain.vblankCounter1 & 3) == 0)
        sprite->x--; // the breeze carries them westward
    x = (sprite->x + sprite->centerToCornerVecX + gSpriteCoordOffsetX) & 0x1FF;
    if (x & 0x100)
        x |= -0x100;
    y = (sprite->y + sprite->centerToCornerVecY + gSpriteCoordOffsetY) & 0xFF;
    if (x < -12)
        sprite->x = 248 - (gSpriteCoordOffsetX + sprite->centerToCornerVecX);
    if (y > 168 && y < 240)
        ResetFallingLeaf(sprite, FALSE);
}

static bool32 FallingLeavesWanted(void)
{
    enum WallySeason season;
    enum WallyRegion region;
    u32 weather;
    if (gMain.callback2 != CB2_Overworld || !MapHasNaturalLight(gMapHeader.mapType)
     || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK) || FlagGet(FLAG_SYS_WEATHER_CTRL))
        return FALSE;
    season = WallyJourney_GetSeason();
    if (season != WALLY_SEASON_AUTUMN && season != WALLY_SEASON_SPRING)
        return FALSE;
    region = GetRegionForMapSec(gMapHeader.regionMapSectionId);
    if (region == WREGION_NONE || region == WREGION_OCEAN || region == WREGION_DESERT)
        return FALSE;
    weather = gWeatherPtr->currWeather;
    return weather == WEATHER_SUNNY || weather == WEATHER_SUNNY_CLOUDS || weather == WEATHER_SHADE;
}

// Called every frame from the overworld.
void WallyJourney_UpdateFallingLeaves(void)
{
    u32 i, count = 0;
    bool32 wanted;
    if ((gMain.vblankCounter1 & 15) != 0)
        return;
    wanted = FallingLeavesWanted();
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == UpdateFallingLeaf)
        {
            if (!wanted)
                DestroySprite(&gSprites[i]);
            else
                count++;
        }
    }
    if (!wanted)
    {
        if (IndexOfSpritePaletteTag(TAG_WALLY_FALLING_LEAF) != 0xFF)
            FreeSpritePaletteByTag(TAG_WALLY_FALLING_LEAF);
        return;
    }
    if (count < FALLING_LEAF_COUNT)
    {
        u8 spriteId;
        if (IndexOfSpritePaletteTag(TAG_WALLY_FALLING_LEAF) == 0xFF)
        {
            struct SpritePalette pal = {
                WallyJourney_GetSeason() == WALLY_SEASON_SPRING ? sFallingLeafPal_Spring : sFallingLeafPal_Autumn,
                TAG_WALLY_FALLING_LEAF
            };
            if (LoadSpritePalette(&pal) == 0xFF)
                return; // no free palette: skip the effect rather than borrow one
        }
        spriteId = CreateSpriteAtEnd(&sFallingLeafTemplate, 0, 0, 77);
        if (spriteId != MAX_SPRITES)
        {
            gSprites[spriteId].coordOffsetEnabled = TRUE;
            ResetFallingLeaf(&gSprites[spriteId], TRUE);
        }
    }
}

#undef tLeafPosY
#undef tLeafSpeedY
#undef tLeafWave
#undef tLeafDrift

// ------------------------------------------------------ townspeople routines
// Outdoor flavour NPCs (see tools/gen_wally_ambient_npcs.py) go home at
// night and shelter from rain, a different mix each day. Story characters,
// trainers, item givers and anyone a script refers to are never on this list,
// so paths, quests and services are never affected.
#include "data/wally_ambient_npcs.h"

bool32 WallyJourney_IsNpcAway(const struct ObjectEventTemplate *template)
{
    u32 i, h, weather, timeOfDay;
    const u8 *script = template->script;
    if (script == NULL || !MapHasNaturalLight(gMapHeader.mapType) || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sWallyAmbientNpcScripts); i++)
    {
        if (sWallyAmbientNpcScripts[i] == script)
            break;
    }
    if (i == ARRAY_COUNT(sWallyAmbientNpcScripts))
        return FALSE;
    timeOfDay = GetTimeOfDay(); // also refreshes gLocalTime
    h = WeatherHash((u32)script ^ ((u32)(WJ_RealClockHour() / 24) * 2246822519u)); // same day for everyone
    if (timeOfDay == TIME_NIGHT && h % 100 < 65)
        return TRUE;                                    // gone home for the night
    if (timeOfDay == TIME_EVENING && h % 100 < 20)
        return TRUE;                                    // headed home early
    weather = GetSavedWeather();
    if ((weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_DOWNPOUR || weather == WEATHER_SNOW)
     && (h >> 8) % 100 < 45)
        return TRUE;                                    // sheltering from the weather
    return FALSE;
}

// ------------------------------------------------- side activity specials
// Contest repeat wins: a rank-appropriate random prize (VAR_0x8004 item, VAR_0x8005 qty).
void WallyJourney_PickContestPrize(void)
{
    static const u16 sNormal[] = {ITEM_POKE_BALL, ITEM_SUPER_POTION, ITEM_REPEL};
    static const u16 sSuper[]  = {ITEM_HEART_SCALE, ITEM_PP_UP, ITEM_NUGGET};
    static const u16 sHyper[]  = {ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_BIG_NUGGET, ITEM_HEART_SCALE};
    static const u16 sMaster[] = {ITEM_PP_MAX, ITEM_RARE_CANDY, ITEM_ABILITY_CAPSULE, ITEM_BOTTLE_CAP, ITEM_STAR_PIECE};
    const u16 *pool; u32 n;
    switch (VarGet(VAR_CONTEST_RANK))
    {
    case CONTEST_RANK_SUPER:  pool = sSuper;  n = ARRAY_COUNT(sSuper);  break;
    case CONTEST_RANK_HYPER:  pool = sHyper;  n = ARRAY_COUNT(sHyper);  break;
    case CONTEST_RANK_MASTER: pool = sMaster; n = ARRAY_COUNT(sMaster); break;
    default:                  pool = sNormal; n = ARRAY_COUNT(sNormal); break;
    }
    gSpecialVar_0x8004 = pool[Random() % n];
    gSpecialVar_0x8005 = (gSpecialVar_0x8004 == ITEM_POKE_BALL) ? 5 : 1;
}

// Battle Frontier "Gimmick Collector": a random Mega Stone or Z-Crystal the
// player doesn't own yet (VAR_0x8005), or ITEM_NONE if they own them all.
// Only stones/crystals that are also hidden in Hoenn, so nothing is shop-only.
void WallyJourney_PickFrontierGimmickItem(void)
{
    static const u16 sPool[] = {
        ITEM_VENUSAURITE,
        ITEM_CHARIZARDITE_X,
        ITEM_CHARIZARDITE_Y,
        ITEM_BLASTOISINITE,
        ITEM_BEEDRILLITE,
        ITEM_PIDGEOTITE,
        ITEM_ALAKAZITE,
        ITEM_SLOWBRONITE,
        ITEM_GENGARITE,
        ITEM_KANGASKHANITE,
        ITEM_PINSIRITE,
        ITEM_GYARADOSITE,
        ITEM_AERODACTYLITE,
        ITEM_AMPHAROSITE,
        ITEM_STEELIXITE,
        ITEM_SCIZORITE,
        ITEM_HERACRONITE,
        ITEM_HOUNDOOMINITE,
        ITEM_TYRANITARITE,
        ITEM_SABLENITE,
        ITEM_MAWILITE,
        ITEM_AGGRONITE,
        ITEM_MEDICHAMITE,
        ITEM_MANECTITE,
        ITEM_SHARPEDONITE,
        ITEM_CAMERUPTITE,
        ITEM_ALTARIANITE,
        ITEM_BANETTITE,
        ITEM_ABSOLITE,
        ITEM_GLALITITE,
        ITEM_SALAMENCITE,
        ITEM_METAGROSSITE,
        ITEM_LOPUNNITE,
        ITEM_GARCHOMPITE,
        ITEM_LUCARIONITE,
        ITEM_ABOMASITE,
        ITEM_GALLADITE,
        ITEM_AUDINITE,
        ITEM_CLEFABLITE,
        ITEM_VICTREEBELITE,
        ITEM_STARMINITE,
        ITEM_DRAGONINITE,
        ITEM_MEGANIUMITE,
        ITEM_FERALIGITE,
        ITEM_SKARMORITE,
        ITEM_FROSLASSITE,
        ITEM_EMBOARITE,
        ITEM_EXCADRITE,
        ITEM_SCOLIPITE,
        ITEM_SCRAFTINITE,
        ITEM_EELEKTROSSITE,
        ITEM_CHANDELURITE,
        ITEM_CHESNAUGHTITE,
        ITEM_DELPHOXITE,
        ITEM_GRENINJITE,
        ITEM_PYROARITE,
        ITEM_FLOETTITE,
        ITEM_MALAMARITE,
        ITEM_BARBARACITE,
        ITEM_DRAGALGITE,
        ITEM_HAWLUCHANITE,
        ITEM_DRAMPANITE,
        ITEM_FALINKSITE,
        ITEM_RAICHUNITE_X,
        ITEM_RAICHUNITE_Y,
        ITEM_CHIMECHITE,
        ITEM_STARAPTITE,
        ITEM_GOLURKITE,
        ITEM_MEOWSTICITE,
        ITEM_CRABOMINITE,
        ITEM_GOLISOPITE,
        ITEM_SCOVILLAINITE,
        ITEM_BAXCALIBRITE,
        ITEM_TATSUGIRINITE,
        ITEM_GLIMMORANITE,
        ITEM_LATIASITE,
        ITEM_LATIOSITE,
        ITEM_NORMALIUM_Z,
        ITEM_FIRIUM_Z,
        ITEM_WATERIUM_Z,
        ITEM_ELECTRIUM_Z,
        ITEM_GRASSIUM_Z,
        ITEM_ICIUM_Z,
        ITEM_FIGHTINIUM_Z,
        ITEM_POISONIUM_Z,
        ITEM_GROUNDIUM_Z,
        ITEM_FLYINIUM_Z,
        ITEM_PSYCHIUM_Z,
        ITEM_BUGINIUM_Z,
        ITEM_ROCKIUM_Z,
        ITEM_GHOSTIUM_Z,
        ITEM_DRAGONIUM_Z,
        ITEM_DARKINIUM_Z,
        ITEM_STEELIUM_Z,
        ITEM_FAIRIUM_Z,
    };
    u16 missing[ARRAY_COUNT(sPool)];
    u32 i, count = 0;
    for (i = 0; i < ARRAY_COUNT(sPool); i++)
        if (!CheckBagHasItem(sPool[i], 1) && !CheckPCHasItem(sPool[i], 1))
            missing[count++] = sPool[i];
    gSpecialVar_0x8005 = count ? missing[Random() % count] : ITEM_NONE;
}

// Catch-rate realism (owner requirement): ordinary species keep their canon
// rates. Legendary and mythical Pokemon are capped at the hardest canon rate (3),
// so Mew, Celebi, Shaymin etc. are as hard as Deoxys or Jirachi. Kyogre and
// Groudon already sit at 3; with only Poke Balls (no Ultra/Timer/Dusk/Master),
// level 75 and Primal Reversion they are far harder than in vanilla Emerald.
// Going below 3 would make them near-impossible because the capture math
// truncates to whole numbers (rate 3 at 1 HP asleep is about 2% per ball).
s32 WallyJourney_AdjustCatchRate(u32 species, s32 catchRate)
{
#if TESTING
    return catchRate;
#else
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 caught;
    if (info->isRestrictedLegendary || info->isSubLegendary || info->isMythical)
        return catchRate > 3 ? 3 : catchRate;
    if (info->isUltraBeast)
        return catchRate;
    // Catch bonus (owner request 2026-10-27): the more of this species caught in total, the easier
    // it is to catch: 6-10 caught x1.1, 11-20 x1.25, 21-30 x1.5, 31+ x2 (capped at 255).
    caught = WallyJourney_CatchCombo(species);
    if (caught >= 31)
        catchRate = catchRate * 2;
    else if (caught >= 21)
        catchRate = catchRate * 3 / 2;
    else if (caught >= 11)
        catchRate = catchRate * 5 / 4;
    else if (caught >= 6)
        catchRate = catchRate * 11 / 10;
    return catchRate > 255 ? 255 : catchRate;
#endif
}

// Postgame titans: Kyogre and Groudon fight with signature movesets and full AI.
void WallyJourney_SetupTitanBattle(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    switch (GetMonData(mon, MON_DATA_SPECIES))
    {
    case SPECIES_KYOGRE:
        SetMonMoveSlot(mon, MOVE_ORIGIN_PULSE, 0);
        SetMonMoveSlot(mon, MOVE_ICE_BEAM, 1);
        SetMonMoveSlot(mon, MOVE_THUNDER, 2);
        SetMonMoveSlot(mon, MOVE_CALM_MIND, 3);
        break;
    case SPECIES_GROUDON:
        SetMonMoveSlot(mon, MOVE_PRECIPICE_BLADES, 0);
        SetMonMoveSlot(mon, MOVE_FIRE_PUNCH, 1);
        SetMonMoveSlot(mon, MOVE_STONE_EDGE, 2);
        SetMonMoveSlot(mon, MOVE_BULK_UP, 3);
        break;
    default:
        break;
    }
#if !IS_FRLG
    FlagSet(FLAG_WALLY_SMART_WILD_AI);
#endif
}

// Game speed options (Options menu). Music is unaffected because it is mixed
// once per hardware frame in the VBlank interrupt; we only run extra logic and
// animation passes inside the frame. Overworld approach after HashtagMarky's
// "Overworld Speedup" tutorial (Team Aqua's Asset Repo wiki); battle uses the
// same idea on the battle callbacks.
static const u8 sOverworldExtra[WALLY_OW_SPEED_COUNT] = {0, 1, 3};
static const u8 sBattleExtra[WALLY_BATTLE_SPEED_COUNT] = {0, 1, 2, 3};

u32 WallyJourney_GetOverworldSpeed(void) { return VarGet(VAR_WALLY_GAME_SPEED) & 0xF; }
u32 WallyJourney_GetBattleSpeed(void) { return (VarGet(VAR_WALLY_GAME_SPEED) >> 4) & 0xF; }

void WallyJourney_SetSpeeds(u32 overworld, u32 battle)
{
    VarSet(VAR_WALLY_GAME_SPEED, (overworld & 0xF) | ((battle & 0xF) << 4));
}

u32 WallyJourney_OverworldExtraIterations(void)
{
    u32 speed = WallyJourney_GetOverworldSpeed();
    // Like Pokemon Emerald Rogue: normal speed during cutscenes/scripts, while
    // field controls are locked, or during a screen fade.
    if (speed >= WALLY_OW_SPEED_COUNT || JOY_HELD(R_BUTTON)
     || ScriptContext_IsEnabled() || ArePlayerFieldControlsLocked() || gPaletteFade.active)
        return 0;
    return sOverworldExtra[speed];
}

u32 WallyJourney_BattleExtraIterations(void)
{
    u32 speed = WallyJourney_GetBattleSpeed();
    if (speed >= WALLY_BATTLE_SPEED_COUNT
     || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK)))
        return 0;
    return sBattleExtra[speed];
}

// Hyper Training NPC: level of party slot gSpecialVar_0x8004 (0 for eggs/empty).
u16 WallyJourney_GetPartyMonLevel(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    if (gSpecialVar_0x8004 >= PARTY_SIZE || GetMonData(mon, MON_DATA_IS_EGG) || !GetMonData(mon, MON_DATA_SPECIES))
        return 0;
    return GetMonData(mon, MON_DATA_LEVEL);
}

// ---------------------------------------------------------------- Mega Stone sparkles
// Owner design (2026-10-23): hidden Mega Stones don't exist until Steven gives the Mega
// Bracelet (engine item ITEM_MEGA_RING). After that, every unclaimed Mega Stone on the
// current map twinkles with Emerald's own sparkle effect (the Abandoned Ship key glint),
// so they can be found by eye like the sparkling stones of the Hoenn remakes.
// Polish (2026-10-26): each stone twinkles on its own beat (phase from its hidden-item id) so
// several stones on screen don't blink in lockstep, each beat is a quick double twinkle so it's
// easy to catch, and nothing sparkles while the screen is fading.
#define MEGA_SPARKLE_PERIOD 80
#define MEGA_SPARKLE_SECOND 9

bool32 WallyJourney_IsHiddenItemWaitingForBracelet(u16 item)
{
    return GetItemHoldEffect(item) == HOLD_EFFECT_MEGA_STONE && !CheckBagHasItem(ITEM_MEGA_RING, 1);
}

void WallyJourney_UpdateMegaStoneSparkles(void)
{
    static u8 sTimer;
    const struct MapEvents *events = gMapHeader.events;
    s16 px, py;
    u32 i;

    if (++sTimer >= MEGA_SPARKLE_PERIOD)
        sTimer = 0;
    if (events == NULL || gPaletteFade.active || !CheckBagHasItem(ITEM_MEGA_RING, 1))
        return;

    PlayerGetDestCoords(&px, &py);
    for (i = 0; i < events->bgEventCount; i++)
    {
        const struct BgEvent *bg = &events->bgEvents[i];
        u32 beat;
        s16 dx, dy;
        if (bg->kind != BG_EVENT_HIDDEN_ITEM)
            continue;
        if (GetItemHoldEffect(bg->bgUnion.hiddenItem.item) != HOLD_EFFECT_MEGA_STONE)
            continue;
        beat = (sTimer + bg->bgUnion.hiddenItem.hiddenItemId * 29) % MEGA_SPARKLE_PERIOD;
        if (beat != 0 && beat != MEGA_SPARKLE_SECOND)
            continue;
        if (FlagGet(bg->bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START))
            continue;
        dx = bg->x + MAP_OFFSET - px;
        dy = bg->y + MAP_OFFSET - py;
        if (dx < -8 || dx > 8 || dy < -6 || dy > 6)
            continue; // only sparkle on screen
        gFieldEffectArguments[0] = bg->x;
        gFieldEffectArguments[1] = bg->y;
        gFieldEffectArguments[2] = 1;
        FieldEffectStart(FLDEFF_SPARKLE);
    }
}

// ---------------------------------------------------------------- Wally walks in and out
// Owner report (2026-10-26): on the route rival battles Wally popped into view two tiles in
// front of the player and popped out of view after two steps. These battles start the moment
// the player enters the map, from whichever side, so there is no fixed entry path. Instead:
// find the nearest walkable tile just off-screen (but still inside the area where objects stay
// loaded) that connects to the tile in front of the player, walk Wally in along that path, and
// after the battle walk him back out the same way before removing him.
#define WJ_PATH_MAX   48
#define WJ_GRID_LEFT  9   // objects stay loaded from 9 tiles left of the player ...
#define WJ_GRID_RIGHT 10  // ... to 10 right,
#define WJ_GRID_UP    7   // 7 up
#define WJ_GRID_DOWN  9   // and 9 down (RemoveObjectEventIfOutsideView)
#define WJ_GRID_W     (WJ_GRID_LEFT + WJ_GRID_RIGHT + 1)
#define WJ_GRID_H     (WJ_GRID_UP + WJ_GRID_DOWN + 1)
#define WJ_GRID_N     (WJ_GRID_W * WJ_GRID_H)

EWRAM_DATA static u8 sWallyInMoves[WJ_PATH_MAX + 2] = {0};
EWRAM_DATA static u8 sWallyOutMoves[WJ_PATH_MAX + 2] = {0};
EWRAM_DATA static bool8 sWallyHasPath = FALSE;
EWRAM_DATA static s16 sWallyTargetX = 0;
EWRAM_DATA static s16 sWallyTargetY = 0;

static const s8 sWJStepX[4] = { 0, 0, -1, 1 };   // down, up, left, right
static const s8 sWJStepY[4] = { 1, -1, 0, 0 };
static const u8 sWJWalk[4] = { MOVEMENT_ACTION_WALK_NORMAL_DOWN, MOVEMENT_ACTION_WALK_NORMAL_UP,
                               MOVEMENT_ACTION_WALK_NORMAL_LEFT, MOVEMENT_ACTION_WALK_NORMAL_RIGHT };
static const u8 sWJFace[4] = { MOVEMENT_ACTION_FACE_DOWN, MOVEMENT_ACTION_FACE_UP,
                               MOVEMENT_ACTION_FACE_LEFT, MOVEMENT_ACTION_FACE_RIGHT };
static const u8 sWJOpposite[4] = { 1, 0, 3, 2 };

static bool32 WJ_TileWalkable(s16 x, s16 y, u8 elevation)
{
    u32 behavior;
    u8 tileElevation;

    if (x < MAP_OFFSET || y < MAP_OFFSET
     || x >= gMapHeader.mapLayout->width + MAP_OFFSET || y >= gMapHeader.mapLayout->height + MAP_OFFSET)
        return FALSE;
    if (MapGridGetCollisionAt(x, y))
        return FALSE;
    tileElevation = MapGridGetElevationAt(x, y);
    if (tileElevation != elevation && tileElevation != 0 && tileElevation != 15 && elevation != 0 && elevation != 15)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsJumpEast(behavior) || MetatileBehavior_IsJumpWest(behavior)
     || MetatileBehavior_IsJumpNorth(behavior) || MetatileBehavior_IsJumpSouth(behavior)
     || MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
     || MetatileBehavior_IsWarpDoor(behavior) || MetatileBehavior_IsNonAnimDoor(behavior))
        return FALSE;
    if (GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    return TRUE;
}

static bool32 WJ_OffScreen(s32 dx, s32 dy)
{
    return dx <= -8 || dx >= 8 || dy <= -6 || dy >= 6;
}

// special: VAR_0x8006/VAR_0x8007 = Wally's start tile (map coords), VAR_RESULT = TRUE if a path was found.
static bool32 WJ_PlanWallyApproach(bool32 fixedStart, s16 startX, s16 startY, bool32 allowOnScreen);
void WallyJourney_PlanWallyApproach(void)
{
    gSpecialVar_Result = WJ_PlanWallyApproach(FALSE, 0, 0, TRUE);
}

// special: same, but Wally starts on the given tile (VAR_0x8006/VAR_0x8007, map coords), e.g. a cave
// exit or door he walks in through; the start tile itself may be a warp.
void WallyJourney_PlanWallyApproachFrom(void)
{
    gSpecialVar_Result = WJ_PlanWallyApproach(TRUE, gSpecialVar_0x8006 + MAP_OFFSET, gSpecialVar_0x8007 + MAP_OFFSET, TRUE);
}

// Breadth-first search from the tile next to the player where Wally will stop (tx, ty).
// Returns the grid index of the tile Wally starts from, or WJ_GRID_N if none.
static u32 WJ_SearchFromTarget(s16 px, s16 py, u8 elevation, s16 tx, s16 ty, bool32 fixedStart, s16 startX, s16 startY,
                               bool32 allowOnScreen, u8 *parentDir, u8 *dist)
{
    u16 queue[WJ_GRID_N];
    u32 head = 0, tail = 0, i, d, far = WJ_GRID_N, n;

    for (i = 0; i < WJ_GRID_N; i++)
        parentDir[i] = 0xFF;
    n = (ty - py + WJ_GRID_UP) * WJ_GRID_W + (tx - px + WJ_GRID_LEFT);
    parentDir[n] = 4;
    dist[n] = 0;
    queue[tail++] = n;
    while (head < tail)
    {
        u32 cur = queue[head++];
        s32 cx = (s32)(cur % WJ_GRID_W) - WJ_GRID_LEFT, cy = (s32)(cur / WJ_GRID_W) - WJ_GRID_UP;
        if (cur != queue[0])
        {
            if (fixedStart)
            {
                if (px + cx == startX && py + cy == startY)
                    return cur;
            }
            else if (WJ_OffScreen(cx, cy))
            {
                return cur;
            }
            if (far == WJ_GRID_N || dist[cur] > dist[far])
                far = cur;
        }
        if (dist[cur] >= WJ_PATH_MAX)
            continue;
        for (d = 0; d < 4; d++)
        {
            s32 nx = cx + sWJStepX[d], ny = cy + sWJStepY[d];
            u32 next;
            if (nx < -WJ_GRID_LEFT || nx > WJ_GRID_RIGHT || ny < -WJ_GRID_UP || ny > WJ_GRID_DOWN)
                continue;
            if (nx == 0 && ny == 0)
                continue; // the player
            next = (ny + WJ_GRID_UP) * WJ_GRID_W + (nx + WJ_GRID_LEFT);
            if (parentDir[next] != 0xFF)
                continue;
            if (!WJ_TileWalkable(px + nx, py + ny, elevation)
             && !(fixedStart && px + nx == startX && py + ny == startY))
                continue;
            parentDir[next] = sWJOpposite[d]; // step from next back toward cur
            dist[next] = dist[cur] + 1;
            queue[tail++] = next;
        }
    }
    // Nothing reachable off-screen (enclosed spot): come from the farthest reachable tile.
    if (!fixedStart && allowOnScreen && far != WJ_GRID_N && dist[far] >= 3)
        return far;
    return WJ_GRID_N;
}

static bool32 WJ_PlanWallyApproach(bool32 fixedStart, s16 startX, s16 startY, bool32 allowOnScreen)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 px = player->currentCoords.x, py = player->currentCoords.y;
    u8 elevation = player->currentElevation;
    u8 parentDir[WJ_GRID_N];     // direction of the step from this tile toward the target; 0xFF = unvisited
    u8 dist[WJ_GRID_N];
    u32 i, d, best = WJ_GRID_N, pass;
    s16 tx = 0, ty = 0;
    static const u8 sTryOrder[4][4] = { {0, 2, 3, 1}, {1, 2, 3, 0}, {2, 0, 1, 3}, {3, 0, 1, 2} };
    u8 facing = player->facingDirection; // DIR_SOUTH=1 .. DIR_EAST=4
    const u8 *order = sTryOrder[(facing >= DIR_SOUTH && facing <= DIR_EAST) ? facing - DIR_SOUTH : 0];

    sWallyHasPath = FALSE;

    // Wally stops on a free tile next to the player, in front of them if possible. A free tile
    // can be a dead end (e.g. a nook beside the player), so try each side in turn: first for a
    // path from off-screen, then (if allowed) for the longest walk inside an enclosed spot.
    for (pass = 0; pass < 2 && best == WJ_GRID_N; pass++)
    {
        if (pass == 1 && (!allowOnScreen || fixedStart))
            break;
        for (i = 0; i < 4 && best == WJ_GRID_N; i++)
        {
            d = order[i];
            tx = px + sWJStepX[d];
            ty = py + sWJStepY[d];
            if (!WJ_TileWalkable(tx, ty, elevation))
                continue;
            best = WJ_SearchFromTarget(px, py, elevation, tx, ty, fixedStart, startX, startY, pass == 1, parentDir, dist);
        }
    }
    if (best == WJ_GRID_N)
        return FALSE; // let the script fall back

    // Walk in: from the start tile follow the parent steps to the target.
    {
        u32 cur = best, steps = 0;
        while (parentDir[cur] != 4 && steps < WJ_PATH_MAX)
        {
            s32 cx = (s32)(cur % WJ_GRID_W) - WJ_GRID_LEFT, cy = (s32)(cur / WJ_GRID_W) - WJ_GRID_UP;
            d = parentDir[cur];
            sWallyInMoves[steps++] = sWJWalk[d];
            cur = (cy + sWJStepY[d] + WJ_GRID_UP) * WJ_GRID_W + (cx + sWJStepX[d] + WJ_GRID_LEFT);
        }
        // face the player at the end
        for (d = 0; d < 4; d++)
            if (tx + sWJStepX[d] == px && ty + sWJStepY[d] == py)
                break;
        sWallyInMoves[steps] = sWJFace[d < 4 ? d : 0];
        sWallyInMoves[steps + 1] = MOVEMENT_ACTION_STEP_END;
        // Walk out: the same path backwards.
        for (i = 0; i < steps; i++)
        {
            u8 in = sWallyInMoves[steps - 1 - i];
            for (d = 0; d < 4; d++)
                if (sWJWalk[d] == in)
                    break;
            sWallyOutMoves[i] = sWJWalk[sWJOpposite[d]];
        }
        sWallyOutMoves[steps] = MOVEMENT_ACTION_STEP_END;
    }
    sWallyTargetX = tx;
    sWallyTargetY = ty;
    gSpecialVar_0x8006 = px + (s32)(best % WJ_GRID_W) - WJ_GRID_LEFT - MAP_OFFSET;
    gSpecialVar_0x8007 = py + (s32)(best / WJ_GRID_W) - WJ_GRID_UP - MAP_OFFSET;
    sWallyHasPath = TRUE;
    return TRUE;
}

// Owner report (2026-10-26): the route rival battles start from an on-frame map script the
// moment the player is on the map. Hold the battle back (the script isn't started, so the player
// keeps control) while Wally could only pop in: while the player is surfing, or standing where no
// path leads in from off-screen (a nook, a gap between buildings). It starts once the player is
// somewhere he can walk in to. Leaving the Mossdeep Gym it also waits for the player's first step.
extern const u8 WallyJourney_EventScript_RivalMeet4[];
extern const u8 WallyJourney_EventScript_RivalMeet5[];
extern const u8 WallyJourney_EventScript_RivalMeet6[];
extern const u8 WallyJourney_EventScript_RivalMeet7[];
EWRAM_DATA static s16 sWallyHoldX = 0;
EWRAM_DATA static s16 sWallyHoldY = 0;
EWRAM_DATA static u8 sWallyHoldElevation = 0;
EWRAM_DATA static bool8 sWallyHoldResult = FALSE;
EWRAM_DATA static const u8 *sWallyHoldScript = NULL;
bool32 WallyJourney_HoldOnFrameScript(const u8 *script)
{
    struct ObjectEvent *player;

    if (script != WallyJourney_EventScript_RivalMeet4 && script != WallyJourney_EventScript_RivalMeet5
     && script != WallyJourney_EventScript_RivalMeet6 && script != WallyJourney_EventScript_RivalMeet7)
        return FALSE;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER))
        return TRUE;
    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (script == WallyJourney_EventScript_RivalMeet7
     && player->currentCoords.x == 38 + MAP_OFFSET && player->currentCoords.y == 10 + MAP_OFFSET)
        return TRUE;
    // The search only changes when the player moves; don't repeat it every frame.
    if (player->currentCoords.x != sWallyHoldX || player->currentCoords.y != sWallyHoldY
     || player->currentElevation != sWallyHoldElevation || script != sWallyHoldScript)
    {
        sWallyHoldX = player->currentCoords.x;
        sWallyHoldY = player->currentCoords.y;
        sWallyHoldElevation = player->currentElevation;
        sWallyHoldScript = script;
        sWallyHoldResult = !WJ_PlanWallyApproach(FALSE, 0, 0, FALSE);
    }
    return sWallyHoldResult;
}

// specialvar: TRUE if Wally walked in along a planned path (so he can walk back out of it).
u16 WallyJourney_WallyHasPath(void)
{
    return sWallyHasPath;
}

// special: Wally (local id in VAR_0x8009) walks the planned path in / back out.
void WallyJourney_StartWallyApproach(void)
{
    ScriptMovement_StartObjectMovementScript(gSpecialVar_0x8009, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWallyInMoves);
}

void WallyJourney_StartWallyLeave(void)
{
    sWallyHasPath = FALSE;
    ScriptMovement_StartObjectMovementScript(gSpecialVar_0x8009, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWallyOutMoves);
}

// special: the player turns to face Wally once he has arrived.
void WallyJourney_FaceWally(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventTurn(player, GetDirectionToFace(player->currentCoords.x, player->currentCoords.y, sWallyTargetX, sWallyTargetY));
}

// ---------------------------------------------------------------- Wally surfs (Sootopolis)
// The 8th Gym stands on an island in Sootopolis's lake, so Wally can only reach the player by
// water: the same surf blob the player rides is attached to his object while he crosses.
EWRAM_DATA static u8 sWallySurfBlobSpriteId = 0;
EWRAM_DATA static bool8 sWallySurfBlobActive = FALSE;

static struct ObjectEvent *WJ_WallyObject(void)
{
    u8 objId;
    if (TryGetObjectEventIdByLocalIdAndMap(gSpecialVar_0x8009, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objId))
        return NULL;
    return &gObjectEvents[objId];
}

static void WJ_CreateWallyBlob(s16 x, s16 y, u8 bobState)
{
    struct ObjectEvent *wally = WJ_WallyObject();
    u8 objId;
    if (wally == NULL)
        return;
    TryGetObjectEventIdByLocalIdAndMap(gSpecialVar_0x8009, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objId);
    gFieldEffectArguments[0] = x;
    gFieldEffectArguments[1] = y;
    gFieldEffectArguments[2] = objId;
    sWallySurfBlobSpriteId = FieldEffectStart(FLDEFF_SURF_BLOB);
    sWallySurfBlobActive = (sWallySurfBlobSpriteId != MAX_SPRITES);
    if (sWallySurfBlobActive)
        SetSurfBlob_BobState(sWallySurfBlobSpriteId, bobState);
}

// special: Wally (VAR_0x8009) is in the water: put him on a surf blob that carries him.
void WallyJourney_WallySurfStart(void)
{
    struct ObjectEvent *wally = WJ_WallyObject();
    if (wally != NULL)
        WJ_CreateWallyBlob(wally->currentCoords.x, wally->currentCoords.y, BOB_PLAYER_AND_MON);
}

// special: he's about to hop onto land: the blob stays behind in the water.
void WallyJourney_WallySurfLeaveBlob(void)
{
    struct ObjectEvent *wally = WJ_WallyObject();
    if (sWallySurfBlobActive)
        SetSurfBlob_BobState(sWallySurfBlobSpriteId, BOB_JUST_MON);
    if (wally != NULL)
        gSprites[wally->spriteId].y2 = 0;
}

// special: he's about to hop into the water one tile below: the blob waits there.
void WallyJourney_WallySurfBlobBelow(void)
{
    struct ObjectEvent *wally = WJ_WallyObject();
    if (wally != NULL)
        WJ_CreateWallyBlob(wally->currentCoords.x, wally->currentCoords.y + 1, BOB_NONE);
}

// special: he has landed on the blob: it carries him from here.
void WallyJourney_WallySurfMount(void)
{
    if (sWallySurfBlobActive)
        SetSurfBlob_BobState(sWallySurfBlobSpriteId, BOB_PLAYER_AND_MON);
}

// special: done with the water: remove the blob.
void WallyJourney_WallySurfEnd(void)
{
    struct ObjectEvent *wally = WJ_WallyObject();
    if (sWallySurfBlobActive)
    {
        DestroySprite(&gSprites[sWallySurfBlobSpriteId]);
        sWallySurfBlobActive = FALSE;
    }
    if (wally != NULL)
        gSprites[wally->spriteId].y2 = 0;
}

// Wally's Journey: on Continue, re-derive every party Pokémon's stats from its species,
// level, nature, IVs and EVs, so nothing edited outside normal play (an old build's
// battle debug menu wrote max HP and moves back to the party) carries over.
// Legit Pokémon come out exactly the same; this only ever undoes edits.
void WallyJourney_SanitizeParty(void)
{
    u32 i, j;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 evTotal = 0;
        if (species == SPECIES_NONE || species >= NUM_SPECIES || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        for (j = 0; j < NUM_STATS; j++)
        {
            u32 iv = GetMonData(mon, MON_DATA_HP_IV + j);
            u32 ev = GetMonData(mon, MON_DATA_HP_EV + j);
            if (iv > MAX_PER_STAT_IVS)
            {
                iv = MAX_PER_STAT_IVS;
                SetMonData(mon, MON_DATA_HP_IV + j, &iv);
            }
            if (ev > MAX_PER_STAT_EVS)
            {
                ev = MAX_PER_STAT_EVS;
                SetMonData(mon, MON_DATA_HP_EV + j, &ev);
            }
            if (evTotal + ev > MAX_TOTAL_EVS)
            {
                ev = MAX_TOTAL_EVS - evTotal;
                SetMonData(mon, MON_DATA_HP_EV + j, &ev);
            }
            evTotal += ev;
        }
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u32 move = GetMonData(mon, MON_DATA_MOVE1 + j);
            if (move >= MOVES_COUNT)
            {
                u32 none = MOVE_NONE, pp = 0;
                SetMonData(mon, MON_DATA_MOVE1 + j, &none);
                SetMonData(mon, MON_DATA_PP1 + j, &pp);
            }
        }
        CalculateMonStats(mon);
    }
}
