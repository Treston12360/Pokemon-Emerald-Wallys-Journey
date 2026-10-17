// Wally's Journey: persistent progression and observed-only rival memory.
// Uses saved Emerald variable IDs; does not inspect the player's hidden party.
#include "global.h"
#include "event_data.h"
#include "wally_journey.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "pokemon.h"
#include "rtc.h"
#include "battle.h"
#include "constants/weather.h"
#include "constants/map_types.h"
#include "constants/maps.h"

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

bool32 WallyJourney_IsExpAllMode(void) { return FlagGet(FLAG_WALLY_EXP_ALL_MODE); }
bool32 WallyJourney_ZMovesEnabled(void) { return !FlagGet(FLAG_WALLY_Z_MOVES_OFF); }

// Script special (after setwildbattle): the Sky Pillar Rayquaza knows Dragon
// Ascent, which is what lets Rayquaza Mega Evolve without a Mega Stone.
void WallyJourney_GiveWildRayquazaDragonAscent(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_RAYQUAZA)
        SetMonMoveSlot(mon, MOVE_DRAGON_ASCENT, 0);
}

// ---------------------------------------------------------------- seasons
enum WallySeason WallyJourney_GetSeason(void)
{
    u32 month = GetMonth();
    if (month >= MONTH_MAR && month <= MONTH_MAY) return WALLY_SEASON_SPRING;
    if (month >= MONTH_JUN && month <= MONTH_AUG) return WALLY_SEASON_SUMMER;
    if (month >= MONTH_SEP && month <= MONTH_NOV) return WALLY_SEASON_AUTUMN;
    if (month == MONTH_DEC || month == MONTH_JAN || month == MONTH_FEB) return WALLY_SEASON_WINTER;
    return WALLY_SEASON_SUMMER; // no or broken clock: plain Hoenn summer, no tint surprises
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
    case WALLY_SEASON_AUTUMN: blend.isTint = TRUE; blend.blendColor = TINT(255, 214, 168); break; // amber leaves
    case WALLY_SEASON_WINTER: blend.isTint = TRUE; blend.blendColor = TINT(214, 226, 255); break; // cool, pale light
    }
    return blend;
}
#undef TINT

// Hoenn is tropical: snow only falls on high ground in winter.
static bool32 IsSnowCountry(void)
{
    u16 map = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    return map == MAP_MT_PYRE_EXTERIOR || map == MAP_MT_PYRE_SUMMIT
        || map == MAP_ROUTE114 || map == MAP_ROUTE115;
}

// Seasonal weather on outdoor maps whose normal weather is clear or cloudy.
// Stable for a whole day per map. Story weather (Groudon/Kyogre crisis,
// scripted maps) is never touched.
u32 WallyJourney_SeasonalWeather(u32 weather)
{
    u32 roll;
    enum MapType type = gMapHeader.mapType;
    if (weather != WEATHER_SUNNY && weather != WEATHER_SUNNY_CLOUDS)
        return weather;
    if (FlagGet(FLAG_SYS_WEATHER_CTRL))
        return weather;
    if (type != MAP_TYPE_ROUTE && type != MAP_TYPE_TOWN && type != MAP_TYPE_CITY)
        return weather;
    RtcCalcLocalTime();
    roll = ((u32)gLocalTime.days * 31u + gSaveBlock1Ptr->location.mapNum * 7u + gSaveBlock1Ptr->location.mapGroup * 13u) % 100;
    switch (WallyJourney_GetSeason())
    {
    case WALLY_SEASON_SPRING:
        return roll < 25 ? WEATHER_RAIN : weather;                       // spring showers
    case WALLY_SEASON_SUMMER:
        return roll < 10 ? WEATHER_RAIN_THUNDERSTORM : weather;          // summer storms
    case WALLY_SEASON_AUTUMN:
        return roll < 30 ? WEATHER_SUNNY_CLOUDS : weather;               // grey autumn days
    case WALLY_SEASON_WINTER:
        if (IsSnowCountry())
            return roll < 60 ? WEATHER_SNOW : weather;                   // mountain snow
        return roll < 15 ? WEATHER_RAIN : weather;                       // cold coastal rain
    }
    return weather;
}
