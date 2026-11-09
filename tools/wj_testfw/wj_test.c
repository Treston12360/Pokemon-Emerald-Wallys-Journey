// TEST-ONLY scaffolding (never committed). Picks a scenario from the RTC day count.
#include "global.h"
#include "rtc.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "item.h"
#include "battle.h"
#include "battle_setup.h"
#include "pokedex.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/trainers.h"
#include "constants/opponents.h"
#include "daycare.h"
#include "safari_zone.h"
#include "coins.h"
#include "random.h"
#include "wj_outfit.h"
#include "constants/wj_outfits.h"

struct WJTestScenario {
    u8 group, num; s16 x, y;
    u16 flags[24];
    u16 vars[8][2];
    u16 party[6][2];
};
#include "wj_test_scenarios.h"

EWRAM_DATA u16 gWJTestEnemy[2][6][2] = {0};
EWRAM_DATA u16 gWJTestInfo[4] = {0};

static u32 ScenarioIndex(void)
{
    struct SiiRtcInfo r;
    RtcGetInfo(&r);
    return RtcGetDayCount(&r) % ARRAY_COUNT(sWJTestScenarios);
}

void WJTest_Warp(void)
{
    const struct WJTestScenario *s = &sWJTestScenarios[ScenarioIndex()];
    SetWarpDestination(s->group, s->num, WARP_ID_NONE, s->x, s->y);
}

void WJTest_ApplyState(void)
{
    u32 i;
    const struct WJTestScenario *s = &sWJTestScenarios[ScenarioIndex()];
    gWJTestInfo[0] = ScenarioIndex();
    for (i = 0; i < ARRAY_COUNT(s->flags) && s->flags[i]; i++)
        FlagSet(s->flags[i]);
    for (i = 0; i < ARRAY_COUNT(s->vars) && s->vars[i][0]; i++)
        VarSet(s->vars[i][0], s->vars[i][1]);
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
    ZeroPlayerPartyMons();
    for (i = 0; i < PARTY_SIZE && s->party[i][0]; i++)
    {
        CreateMon(&gPlayerParty[i], s->party[i][0], s->party[i][1], 0, OTID_STRUCT_PLAYER_ID);
        GiveMonInitialMoveset(&gPlayerParty[i]);
        CalculateMonStats(&gPlayerParty[i]);
        CalculateMonStats(&gPlayerParty[i]);
        SetMonData(&gPlayerParty[i], MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
        { u8 ml = 5; SetMonData(&gPlayerParty[i], MON_DATA_MET_LEVEL, &ml); }
        if (s->party[i][1] >= 90) { u8 bon = 0xFF; SetMonMoveSlot(&gPlayerParty[i], MOVE_FLASH_CANNON, 0); SetMonData(&gPlayerParty[i], MON_DATA_PP_BONUSES, &bon); { u8 pp = 16; SetMonData(&gPlayerParty[i], MON_DATA_PP1, &pp); } }
        { u32 hp = GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP); SetMonData(&gPlayerParty[i], MON_DATA_HP, &hp); }
#ifdef WJ_ALMOST
        // TEST ONLY ("almost": true): one EXP point short of the next level
        { u32 sp = s->party[i][0], lv = s->party[i][1];
          u32 exp = gExperienceTables[gSpeciesInfo[sp].growthRate][lv + 1] - 1;
          SetMonData(&gPlayerParty[i], MON_DATA_EXP, &exp);
          SetMonMoveSlot(&gPlayerParty[i], MOVE_THUNDERBOLT, 0); }
#endif
    }
#ifdef WJ_ALMOST
    AddBagItem(ITEM_RARE_CANDY, 5);
    AddBagItem(ITEM_EXP_ALL, 1);
    AddBagItem(ITEM_POKE_BALL, 20);
#endif
    CalculatePlayerPartyCount();
#ifdef WJ_DEXDEMO
    { u32 sp; for (sp = 1; sp < NUM_SPECIES; sp++) { if (!IsSpeciesEnabled(sp) || !SpeciesToNationalPokedexNum(sp)) continue; GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_SEEN); if (sp <= 386 && sp % 3 != 0) GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_CAUGHT); } }
#endif
    AddBagItem(ITEM_MAX_REPEL, 99);
    { static const u16 hm[] = {ITEM_HM_CUT, ITEM_HM_FLASH, ITEM_HM_ROCK_SMASH, ITEM_HM_STRENGTH, ITEM_HM_SURF, ITEM_HM_FLY, ITEM_HM_DIVE, ITEM_HM_WATERFALL, ITEM_Z_POWER_RING, ITEM_MEGA_RING};
      for (i = 0; i < ARRAY_COUNT(hm); i++) AddBagItem(hm[i], 1); }
#ifdef WJ_HAS_EXTRA
    WJTest_ScenarioExtra(ScenarioIndex());
#endif
}

void WJTest_RecordEnemy(void)
{
    u32 t, i;
    for (t = 0; t < 2; t++)
        for (i = 0; i < PARTY_SIZE; i++)
        {
            gWJTestEnemy[t][i][0] = GetMonData(&gParties[B_TRAINER_OPPONENT_A + t][i], MON_DATA_SPECIES);
            gWJTestEnemy[t][i][1] = GetMonData(&gParties[B_TRAINER_OPPONENT_A + t][i], MON_DATA_LEVEL);
        }
#ifdef WJ_WEAK
    // TEST ONLY: every opposing Pokemon is level 1 so the test party can win.
    for (t = 0; t < 2; t++)
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gParties[B_TRAINER_OPPONENT_A + t][i], MON_DATA_SPECIES) != SPECIES_NONE)
            {
                struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A + t][i];
                u32 lvl = 1, exp = 0;
                SetMonData(mon, MON_DATA_LEVEL, &lvl);
                SetMonData(mon, MON_DATA_EXP, &exp);
                CalculateMonStats(mon);
            }
#endif
    gWJTestInfo[1] = TRAINER_BATTLE_PARAM.opponentA;
    gWJTestInfo[2] = TRAINER_BATTLE_PARAM.opponentB;
}
