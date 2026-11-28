// PC build only: things the GBA build gets from its linker script, crt0.s,
// rom_header.s or the GBA BIOS.
#include "global.h"
#include "main.h"

extern void CB2_InitCopyrightScreenAfterBootup(void);

// ld_script_modern.ld: gInitialMainCB2 = CB2_InitCopyrightScreenAfterBootup;
void gInitialMainCB2(void)
{
    CB2_InitCopyrightScreenAfterBootup();
}

// crt0.s: re-copies initialised EWRAM data after a failed link. On PC the
// globals keep their values, so there is nothing to restore.
void ReInitializeEWRAM(void)
{
}

// rom_header.s
const u8 RomHeaderGameCode[GAME_CODE_LENGTH] = {'B', 'P', 'E', 'E'};
const u8 RomHeaderSoftwareVersion = 0;

// assertf.c measures free stack against the end of IWRAM; give it a big
// enough fake gap that it always takes the "plenty of stack" path.
char __iwram_end[1];

// GBA BIOS SWI 0x06
s32 Div(s32 num, s32 denom)
{
    return denom ? num / denom : 0;
}

// GBA BIOS SWI 0x10: expand packed pixels (1/2/4/8 bit -> 1/2/4/8/16/32 bit).
struct BitUnPackParams
{
    u16 srcLength;
    u8 srcWidth;
    u8 destWidth;
    u32 offset;
};

void BitUnPack(const void *src, void *dest, const void *paramsVoid)
{
    const struct BitUnPackParams *params = paramsVoid;
    const u8 *in = src;
    u32 *out = dest;
    u32 srcWidth = params->srcWidth;
    u32 destWidth = params->destWidth;
    u32 base = params->offset & 0x7FFFFFFF;
    bool32 zeroData = (params->offset >> 31) & 1;
    u32 outWord = 0, outBits = 0;
    u32 i, bit;

    for (i = 0; i < params->srcLength; i++)
    {
        u8 byte = in[i];
        for (bit = 0; bit < 8; bit += srcWidth)
        {
            u32 value = (byte >> bit) & ((1u << srcWidth) - 1);
            if (value || zeroData)
                value += base;
            outWord |= value << outBits;
            outBits += destWidth;
            if (outBits >= 32)
            {
                *out++ = outWord;
                outWord = 0;
                outBits = 0;
            }
        }
    }
}

// Test aid (sdl2.c "enemymoves"): a battler's live moves, full PP.
#include "battle.h"
#include "event_data.h"
#include "battle_setup.h"
#include "save.h"
#include "field_move.h"
// Test aid: WJPC_TEST_NO_BATTLES=1 - no wild encounters and no trainers spotting the player, so a long
// walking/warping test script is not interrupted (a test `warp` in the middle of a battle crashes: the
// battle's memory is freed by the map load while the game still says it is in a battle).
extern char *getenv(const char *name);
bool32 WJPC_TestNoBattles(void)
{
    static int sOn = -1;
    if (sOn < 0)
        sOn = getenv("WJPC_TEST_NO_BATTLES") != NULL;
    return sOn;
}

// "state learnscreen": what a TM does after the screen - the chosen slot gets the new move - then the field.
#include "pokemon_summary_screen.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
static u16 sTestLearnMove;
static void TestLearnScreenDone(void)
{
    u8 slot = GetMoveSlotToReplace();
    if (slot < MAX_MON_MOVES)
    {
        RemoveMonPPBonus(&gParties[B_TRAINER_PLAYER][0], slot);
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], sTestLearnMove, slot);
    }
    SetMainCallback2(CB2_ReturnToField);
}

// Test aid: WJPC_TEST_AUTO_REEL=1 - every fishing bite is reeled in as if A was pressed in time (POL-35).
bool32 WJPC_TestAutoReel(void)
{
    return getenv("WJPC_TEST_AUTO_REEL") != NULL;
}

// Test aid (sdl2.c WJPC_TEST_WINDOW_NOBATTLE): in a battle.
bool32 WJPC_TestInBattle(void)
{
    return gMain.inBattle;
}

// Test aid (sdl2.c "state"): game state read or written from a test script.
int WJPC_TestState(const char *what, int id, int set, int value)
{
    if (!strcmp(what, "var"))
    {
        if (set)
            VarSet(id, value);
        return VarGet(id);
    }
    if (!strcmp(what, "flag"))
    {
        if (set && value)
            FlagSet(id);
        else if (set)
            FlagClear(id);
        return FlagGet(id);
    }
    if (!strcmp(what, "trainer"))
    {
        if (set && value)
            SetTrainerFlag(id);
        else if (set)
            ClearTrainerFlag(id);
        return HasTrainerBeenFought(id);
    }
    if (!strcmp(what, "save"))
        return TrySavingData(SAVE_NORMAL);
    if (!strcmp(what, "hp"))
    {
        // "state hp SLOT [VALUE]": that party Pokemon's HP, set first if VALUE is given (test aid, Project Tandem T6:
        // battles with a friend put the party back as it was)
        if (id < 0 || id >= PARTY_SIZE)
            return -1;
        if (set)
        {
            u16 hp = value;
            SetMonData(&gParties[B_TRAINER_PLAYER][id], MON_DATA_HP, &hp);
        }
        return GetMonData(&gParties[B_TRAINER_PLAYER][id], MON_DATA_HP);
    }
    if (!strcmp(what, "party"))
    {
        // "state party SLOT": that party Pokemon's species, 0 if none (test aid, Project Tandem T5: after a trade)
        if (id < 0 || id >= PARTY_SIZE)
            return -1;
        return GetMonData(&gParties[B_TRAINER_PLAYER][id], MON_DATA_SPECIES);
    }
    if (!strcmp(what, "rematch"))
    {
        // "state rematch TRAINER [0/1]": that first-battle trainer's rematch is ready (Match Call; test aid, Project
        // Tandem T6b: team-up rematches)
        extern s32 FirstBattleTrainerIdToRematchTableId(const struct RematchTrainer *table, u16 trainerId);
        s32 t = FirstBattleTrainerIdToRematchTableId(gRematchTable, id);
        if (t < 0)
            return -1;
        if (set)
            gSaveBlock1Ptr->trainerRematches[t] = value;
        {
            extern bool8 ShouldTryRematchBattleForTrainerId(u16 trainerId);
            FILE *f = fopen("state.log", "a");
            if (f != NULL)
            {
                extern bool8 ShouldTryRematchBattle(void);
                fprintf(f, "  (rematch table %d, should try %d; battle param opponent %u: %d)\n", (int)t,
                        ShouldTryRematchBattleForTrainerId(id), TRAINER_BATTLE_PARAM.opponentA, ShouldTryRematchBattle());
                fclose(f);
            }
        }
        return gSaveBlock1Ptr->trainerRematches[t];
    }
    if (!strcmp(what, "money"))
    {
        // "state money 0 [VALUE]": the player's money, set first if VALUE is given (test aid, Project Tandem T6b: a
        // lost team-up costs it; S2: tells saves apart)
        extern u32 GetMoney(u32 *moneyPtr);
        extern void SetMoney(u32 *moneyPtr, u32 newValue);
        if (set)
            SetMoney(&gSaveBlock1Ptr->money, value);
        return GetMoney(&gSaveBlock1Ptr->money);
    }
    if (!strcmp(what, "level") || !strcmp(what, "exp") || !strcmp(what, "move"))
    {
        // "state level SLOT" / "state exp SLOT": that party Pokemon's level or Exp. points; "state move N": party
        // Pokemon N/4's move N%4 (test aid, Project Tandem T6b: the Exp. after a team-up)
        if (!strcmp(what, "move"))
            return (id >= 0 && id < PARTY_SIZE * MAX_MON_MOVES) ? (int)GetMonData(&gParties[B_TRAINER_PLAYER][id / MAX_MON_MOVES], MON_DATA_MOVE1 + id % MAX_MON_MOVES) : -1;
        if (id < 0 || id >= PARTY_SIZE)
            return -1;
        return GetMonData(&gParties[B_TRAINER_PLAYER][id], !strcmp(what, "level") ? MON_DATA_LEVEL : MON_DATA_EXP);
    }
    if (!strcmp(what, "setmove"))
    {
        // "state setmove SLOT MOVE": the first party Pokemon's move in that slot (test aid, POL-25).
        if (set && id >= 0 && id < MAX_MON_MOVES)
        {
            u16 move = value;
            SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1 + id, &move);
        }
        return GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1 + (id & 3));
    }
    if (!strcmp(what, "learnscreen"))
    {
        // "state learnscreen MOVE": from the field, open the "which move should be forgotten?" screen for the
        // first party Pokemon learning MOVE, as a TM or level-up does (test aid, POL-25).
        extern void ShowSelectMovePokemonSummaryScreen(struct Pokemon *mons, u8 monIndex, void (*callback)(void), u16 newMove);
        sTestLearnMove = id;
        ShowSelectMovePokemonSummaryScreen(gParties[B_TRAINER_PLAYER], 0, TestLearnScreenDone, id);
        return id;
    }
    if (!strcmp(what, "classicgfx"))
    {
        // "state classicgfx FEATURE [0/1]": Options -> graphics style (WJGFX_*), 1 = Classic (test aid)
        extern bool32 WJ_ClassicGfx(u32 feature);
        if (set && gSaveBlock3Ptr != NULL)
        {
            gSaveBlock3Ptr->wjGfxMagic = 0xA5; // (WJ_GFX_MAGIC, wally_journey.c)
            if (value)
                gSaveBlock3Ptr->wjClassicGfx |= 1u << id;
            else
                gSaveBlock3Ptr->wjClassicGfx &= ~(1u << id);
        }
        return WJ_ClassicGfx(id);
    }
    if (!strcmp(what, "reg"))
    {
        // "state reg OFFSET": a GBA I/O register's value (16 bits), e.g. 0 = DISPCNT, 8.. = BG0CNT.. (test aid)
        return *(volatile u16 *)(REG_BASE + (id & 0x3FE));
    }
    if (!strcmp(what, "storage"))
    {
        // "state storage 0": from the field, open the PC's Pokemon storage menu as the PC does - Sword/Shield
        // or classic boxes per the player's setting (test aid, POL-32).
        extern void ShowPokemonStorageSystemPC(void);
        ShowPokemonStorageSystemPC();
        return 1;
    }
    if (!strcmp(what, "boxmon"))
    {
        // "state boxmon BOX*30+SLOT": species in that PC box slot, 0 = empty (test aid, POL-32)
        return GetBoxMonDataAt(id / IN_BOX_COUNT, id % IN_BOX_COUNT, MON_DATA_SPECIES);
    }
    if (!strcmp(what, "rng"))
    {
        // "state rng SEED": reseed the game's random numbers, so a test run repeats exactly (test aid, POL-35)
        extern void SeedRng(u32 seed);
        SeedRng(id);
        return id;
    }
    if (!strcmp(what, "fish"))
    {
        // "state fish ROD": from the field, cast that rod (0 old, 1 good, 2 super) as the bag does (test aid, POL-35)
        extern void StartFishing(u8 rod);
        StartFishing(id);
        return id;
    }
    if (!strcmp(what, "fieldmove"))
    {
        // "state fieldmove ID": can the player use this field move here (badge + HM item)? (Project ARIA test aid)
        extern bool32 WallyHasFieldMoveLicense(enum FieldMove fieldMove);
        return WallyHasFieldMoveLicense(id);
    }
    return -1;
}

void WJPC_TestSetBattlerHp(int battler, int hp)
{
    if (battler >= 0 && battler < MAX_BATTLERS_COUNT)
        gBattleMons[battler].hp = hp;
}

void WJPC_TestSetBattlerMoves(int battler, const int *moves)
{
    for (int k = 0; k < MAX_MON_MOVES; k++)
    {
        gBattleMons[battler].moves[k] = moves[k];
        gBattleMons[battler].pp[k] = 10;
    }
}

// Test aid (Project ARIA K6): open the Fly map from the field.
void WJPC_TestOpenFlyMap(void)
{
    extern void CB2_OpenFlyMap(void);
    extern void CleanupOverworldWindowsAndTilemaps(void);
    CleanupOverworldWindowsAndTilemaps(); // as Crossroads' Flight Call special does
    gMain.state = 0;
    SetMainCallback2(CB2_OpenFlyMap);
}
