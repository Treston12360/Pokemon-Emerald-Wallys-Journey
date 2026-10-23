#ifndef GUARD_WALLY_JOURNEY_H
#define GUARD_WALLY_JOURNEY_H
#include "global.h"
#include "palette.h"
void WallyJourney_Reset(void);
void WallyJourney_OnMapEntered(u8 mapGroup, u8 mapNum);
u16 WallyJourney_GetPeakBadges(void);
u16 WallyJourney_GetDiscoveredBiomes(void);
u16 WallyJourney_GetSuggestedAceLevel(void);
u16 WallyJourney_GetConfidence(void);
void WallyJourney_RecordRivalBattle(u8 playerWon);
u16 WallyJourney_GetWallyDefeats(void);
u16 WallyJourney_GetWallyVictories(void);
void WallyJourney_RecordObservedSpecies(u16 species);
u16 WallyJourney_GetLastObservedSpecies(void);
u16 WallyJourney_GetPreviousObservedSpecies(void);
bool8 WallyJourney_ShouldLeadWithMagneton(u16 primaryType,u16 secondaryType);
// Poké Ball-only rule: any other capture ball the player would receive becomes a Poké Ball.
bool32 WallyJourney_IsForbiddenBall(u16 itemId);
u16 WallyJourney_ConvertForbiddenBall(u16 itemId);
// Options-menu toggles (saved as event flags, so they persist through save/load).
bool32 WallyJourney_IsExpAllMode(void);
void WallyJourney_SyncExpItem(void);
bool32 WallyJourney_ZMovesEnabled(void);
void WallyJourney_GiveWildRayquazaDragonAscent(void);
// Seasons: 48 real hours each, spring -> summer -> autumn -> winter (RTC).
enum WallySeason { WALLY_SEASON_SPRING, WALLY_SEASON_SUMMER, WALLY_SEASON_AUTUMN, WALLY_SEASON_WINTER };
enum WallySeason WallyJourney_GetSeason(void);
struct BlendSettings WallyJourney_GetSeasonDayBlend(void);
u32 WallyJourney_SeasonalWeather(u32 weather);
void WallyJourney_UpdateLiveWeather(void);
bool32 WallyJourney_TryRegionalCoordWeather(u32 coordWeather);
void WallyJourney_TrySeasonNotice(void);
void WallyJourney_UpdateFallingLeaves(void);
void WallyJourney_ApplySeasonalFoliage(u32 palStart, u32 palCount, bool32 alsoFaded, bool32 isPrimary);
bool32 WallyJourney_PrimaryFoliageStale(void);
struct ObjectEventTemplate;
bool32 WallyJourney_IsNpcAway(const struct ObjectEventTemplate *template);
bool32 WallyJourney_GetForecast(u32 mapSec, u8 *now, u8 *later);
void WallyJourney_PickContestPrize(void);
void WallyJourney_PickFrontierGimmickItem(void);
s32 WallyJourney_AdjustCatchRate(u32 species, s32 catchRate);
void WallyJourney_SetupTitanBattle(void);
#define WALLY_OW_SPEED_COUNT 3      // 1x, 2x, 4x
#define WALLY_BATTLE_SPEED_COUNT 4  // 1x, 2x, 3x, 4x
u32 WallyJourney_GetOverworldSpeed(void);
u32 WallyJourney_GetBattleSpeed(void);
void WallyJourney_SetSpeeds(u32 overworld, u32 battle);
u32 WallyJourney_OverworldExtraIterations(void);
u32 WallyJourney_BattleExtraIterations(void);
u16 WallyJourney_GetPartyMonLevel(void);
#endif
