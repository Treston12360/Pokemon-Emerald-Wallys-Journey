#ifndef GUARD_WALLY_JOURNEY_H
#define GUARD_WALLY_JOURNEY_H
#include "global.h"
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
bool32 WallyJourney_ZMovesEnabled(void);
void WallyJourney_GiveWildRayquazaDragonAscent(void);
#endif
