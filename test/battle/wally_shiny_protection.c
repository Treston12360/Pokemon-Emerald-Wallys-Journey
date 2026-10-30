#include "global.h"
#include "event_data.h"
#include "test/battle.h"

// Wally's Journey (owner request 2026-10-29): a wild shiny can only be knocked out by itself.

WILD_BATTLE_TEST("Wally: a wild shiny can't be knocked out by the player's attack")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE) { Shiny(TRUE); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        NOT MESSAGE("The wild Caterpie fainted!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

WILD_BATTLE_TEST("Wally: a wild shiny can't be knocked out by a one-hit KO move")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Level(100); }
        OPPONENT(SPECIES_CATERPIE) { Shiny(TRUE); Level(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_GUILLOTINE, hit: TRUE); }
    } SCENE {
        NOT MESSAGE("The wild Caterpie fainted!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

WILD_BATTLE_TEST("Wally: a wild shiny can't be knocked out by poison at the end of the turn")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE) { Shiny(TRUE); HP(1); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { }
    } SCENE {
        NOT MESSAGE("The wild Caterpie fainted!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

WILD_BATTLE_TEST("Wally: a wild shiny can knock itself out with recoil")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE) { Shiny(TRUE); HP(1); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_DOUBLE_EDGE); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    }
}

WILD_BATTLE_TEST("Wally: an ordinary wild Pokemon can still be knocked out")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE) { Shiny(FALSE); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    }
}

// Wally's Journey (owner request 2026-10-29): LEVEL CAP option, no EXP at the cap.
WILD_BATTLE_TEST("Wally: with LEVEL CAP on, a Pokemon at the cap gains no EXP")
{
    u32 expBefore = 0;
    GIVEN {
        FlagClear(FLAG_BADGE01_GET);
        FlagSet(FLAG_WALLY_LEVEL_CAP); // cap 15 before the first badge
        PLAYER(SPECIES_WOBBUFFET) { Level(15); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    } THEN {
        expBefore = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][15];
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), expBefore);
        FlagClear(FLAG_WALLY_LEVEL_CAP);
    }
}

WILD_BATTLE_TEST("Wally: with LEVEL CAP off, the same Pokemon gains EXP")
{
    u32 expBefore = 0;
    GIVEN {
        FlagClear(FLAG_WALLY_LEVEL_CAP);
        PLAYER(SPECIES_WOBBUFFET) { Level(15); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    } THEN {
        expBefore = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][15];
        EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), expBefore);
    }
}
