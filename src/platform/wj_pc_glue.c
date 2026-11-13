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
