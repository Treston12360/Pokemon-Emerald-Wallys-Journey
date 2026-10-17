// Wally's Journey: persistent progression and observed-only rival memory.
// Uses saved Emerald variable IDs; does not inspect the player's hidden party.
#include "global.h"
#include "main.h"
#include "palette.h"
#include "script.h"
#include "event_data.h"
#include "wally_journey.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "pokemon.h"
#include "rtc.h"
#include "battle.h"
#include "item.h"
#include "random.h"
#include "constants/contest.h"
#include "constants/weather.h"
#include "constants/map_types.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "field_weather.h"
#include "overworld.h"

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
// Owner design (2026-10-17): each season lasts 48 real hours, cycling
// spring -> summer -> autumn -> winter every 8 days. Driven by the real-time
// clock, so time keeps passing while the game is off and the season is the
// same on every map and after every reload.
#define WALLY_HOURS_PER_SEASON 48
enum WallySeason WallyJourney_GetSeason(void)
{
    s32 hours;
    if (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK)
        return WALLY_SEASON_SUMMER; // no or broken clock: plain Hoenn summer, no tint surprises
    RtcCalcLocalTime();
    hours = gLocalTime.days * 24 + gLocalTime.hours;
    if (hours < 0)
        hours = 0;
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
    case WALLY_SEASON_AUTUMN: blend.isTint = TRUE; blend.blendColor = TINT(255, 214, 168); break; // amber leaves
    case WALLY_SEASON_WINTER: blend.isTint = TRUE; blend.blendColor = TINT(214, 226, 255); break; // cool, pale light
    }
    return blend;
}
#undef TINT

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
    WREGION_HIGHLAND,    // Fallarbor, Routes 112, 114, 115, Mt. Pyre
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
    case MAPSEC_MT_PYRE:
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

// Hoenn is tropical: snow only falls on high ground in winter.
static bool32 IsSnowCountry(void)
{
    u16 map = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    return map == MAP_MT_PYRE_EXTERIOR || map == MAP_MT_PYRE_SUMMIT
        || map == MAP_ROUTE112 || map == MAP_ROUTE114 || map == MAP_ROUTE115;
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
    return (hour / WALLY_HOURS_PER_SEASON) % 4;
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
        if (phase == 0 || phase == (s32)length - 1)
            w = WEATHER_SUNNY_CLOUDS;                       // clouds gather / skies clear
        else if (phase == 1 && strength != FRONT_SHOWERS)
            w = WEATHER_SHADE;                              // skies darken
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
            w = WEATHER_SUNNY_CLOUDS;                       // grey autumn day
    }

    if (region == WREGION_DESERT && w != fairWeather)
        w = WEATHER_SUNNY_CLOUDS;                           // fronts pass over the desert dry
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
    RtcCalcLocalTime();
    return gLocalTime.days * 24 + gLocalTime.hours;
}

// Called when a map's weather is loaded from its header.
u32 WallyJourney_SeasonalWeather(u32 weather)
{
    u32 header = gMapHeader.weather;
    enum MapType type = gMapHeader.mapType;
    enum WallyRegion region = GetRegionForMapSec(gMapHeader.regionMapSectionId);
    u32 fair, w;

    sAutoWeatherPlus1 = 0;
    if (header != WEATHER_SUNNY && header != WEATHER_SUNNY_CLOUDS
     && header != WEATHER_ROUTE119_CYCLE && header != WEATHER_ROUTE123_CYCLE)
        return weather;
    if (FlagGet(FLAG_SYS_WEATHER_CTRL))
        return weather;
    if (type != MAP_TYPE_ROUTE && type != MAP_TYPE_TOWN && type != MAP_TYPE_CITY && type != MAP_TYPE_OCEAN_ROUTE)
        return weather;
    if (region == WREGION_NONE || (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK))
        return weather;

    fair = (header == WEATHER_SUNNY_CLOUDS) ? WEATHER_SUNNY_CLOUDS : WEATHER_SUNNY;
    sAutoFairWeather = fair;
    w = RegionWeatherAt(region, CurrentRealHour(), fair, IsSnowCountry());
    sAutoWeatherPlus1 = w + 1;
    return w;
}

// Called about once a minute in the overworld: lets weather change while the
// player stands on a map, fading in and out through the normal weather engine.
void WallyJourney_UpdateLiveWeather(void)
{
    u32 w;
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
    if ((info->isRestrictedLegendary || info->isSubLegendary || info->isMythical) && catchRate > 3)
        return 3;
    return catchRate;
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
    FlagSet(FLAG_WALLY_SMART_WILD_AI);
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
