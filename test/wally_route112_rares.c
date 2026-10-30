#include "global.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/maps.h"
#include "constants/rtc.h"

// Owner question 2026-10-29: can Hisuian Growlithe (and the other Route 112 rare visitors) really
// be found? Generates real grass encounters on Route 112 for every time-of-day table and counts.
#define ENCOUNTERS_PER_TABLE 1500

TEST("Wally: Route 112 grass can give Hisuian Growlithe, Chimchar and Charmander")
{
    u32 t, n, tables = 0;
    u32 growlithe = 0, chimchar = 0, charmander = 0, total = 0;
    u16 headerId;

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE112);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE112);
    headerId = GetCurrentMapWildMonHeaderId();
    EXPECT_NE(headerId, HEADER_NONE);

    for (t = 0; t < TIMES_OF_DAY_COUNT; t++)
    {
        const struct WildPokemonInfo *info = gWildMonHeaders[headerId].encounterTypes[t].landMonsInfo;
        u32 g = 0, ch = 0, cm = 0;
        if (info == NULL)
            continue;
        tables++;
        for (n = 0; n < ENCOUNTERS_PER_TABLE; n++)
        {
            u16 species;
            if (!TryGenerateWildMon(info, WILD_AREA_LAND, 0))
                continue;
            total++;
            species = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
            if (species == SPECIES_GROWLITHE_HISUI) g++;
            else if (species == SPECIES_CHIMCHAR) ch++;
            else if (species == SPECIES_CHARMANDER) cm++;
        }
        Test_MgbaPrintf("Route 112 table %d: Hisuian Growlithe %d, Chimchar %d, Charmander %d of %d", t, g, ch, cm, ENCOUNTERS_PER_TABLE);
        EXPECT_GT(g, 0);
        growlithe += g; chimchar += ch; charmander += cm;
    }
    Test_MgbaPrintf("Route 112 total: Hisuian Growlithe %d, Chimchar %d, Charmander %d of %d encounters (%d tables)",
                    growlithe, chimchar, charmander, total, tables);
    EXPECT_GT(tables, 0);
    // 0.5% and 1% of the encounters, with a wide margin for randomness.
    EXPECT_GT(growlithe * 1000, total * 3);  // above 0.3%
    EXPECT_LT(growlithe * 1000, total * 7);  // below 0.7%
    EXPECT_GT(chimchar * 1000, total * 7);   // above 0.7%
    EXPECT_GT(charmander * 1000, total * 7); // above 0.7%
}
