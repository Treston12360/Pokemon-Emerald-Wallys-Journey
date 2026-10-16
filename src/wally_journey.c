// Wally's Journey: persistent progression and observed-only rival memory.
// Uses saved Emerald variable IDs; does not inspect the player's hidden party.
#include "global.h"
#include "event_data.h"
#include "wally_journey.h"

#define WALLY_VAR_PEAK_BADGES   VAR_UNUSED_0x40F7
#define WALLY_VAR_BIOME_MASK    VAR_UNUSED_0x40F8
#define WALLY_VAR_WALLY_DEFEATS VAR_UNUSED_0x40F9
#define WALLY_VAR_WALLY_WINS    VAR_UNUSED_0x40FA
#define WALLY_VAR_LAST_SPECIES  VAR_UNUSED_0x40FB
#define WALLY_VAR_PREV_SPECIES  VAR_UNUSED_0x40FC

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
}
void WallyJourney_OnMapEntered(u8 group,u8 num) {
    u16 badges = CountBadges();
    u16 known = VarGet(WALLY_VAR_BIOME_MASK), bit = AreaBit(group,num);
    if (badges > VarGet(WALLY_VAR_PEAK_BADGES))
        VarSet(WALLY_VAR_PEAK_BADGES,badges);
    if (bit && !(known & bit))
        VarSet(WALLY_VAR_BIOME_MASK,known | bit);
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
