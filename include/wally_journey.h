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
#endif
