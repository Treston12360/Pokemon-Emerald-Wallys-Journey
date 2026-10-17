// Wally's Journey: persistent progression and observed-only rival memory.
// Uses saved Emerald variable IDs; does not inspect the player's hidden party.
#include "global.h"
#include "main.h"
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
    if (speed >= WALLY_OW_SPEED_COUNT || JOY_HELD(R_BUTTON))
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
