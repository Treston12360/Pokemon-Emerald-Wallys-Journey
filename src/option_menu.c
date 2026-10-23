#include "global.h"
#include "wally_journey.h"
#include "option_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "event_data.h"
#include "constants/rgb.h"

#define tMenuSelection data[0]
#define tTextSpeed data[1]
#define tBattleSceneOff data[2]
#define tBattleStyle data[3]
#define tSound data[4]
#define tButtonMode data[5]
#define tWindowFrameType data[6]
#define tExpAll data[7]
#define tZMovesOff data[8]
#define tGameSpeed data[9]
#define tBattleSpeed data[10]
#define tWildMonsOff data[11]

enum
{
    MENUITEM_TEXTSPEED,
    MENUITEM_BATTLESCENE,
    MENUITEM_BATTLESTYLE,
    MENUITEM_SOUND,
    MENUITEM_BUTTONMODE,
    MENUITEM_FRAMETYPE,
    MENUITEM_EXPMODE,
    MENUITEM_ZMOVES,
    MENUITEM_GAMESPEED,
    MENUITEM_BATTLESPEED,
    MENUITEM_WILDMONS,
    MENUITEM_CANCEL,
    MENUITEM_COUNT,
};

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

// Wally's Journey: the list now has more rows than fit on screen, so it scrolls.
#define VISIBLE_ITEMS     7
#define ITEM_Y(item)      (((item) - sTopItem) * 16)
#define ITEM_VISIBLE(item) ((item) >= sTopItem && (item) < sTopItem + VISIBLE_ITEMS)
#define YPOS_TEXTSPEED    ITEM_Y(MENUITEM_TEXTSPEED)
#define YPOS_BATTLESCENE  ITEM_Y(MENUITEM_BATTLESCENE)
#define YPOS_BATTLESTYLE  ITEM_Y(MENUITEM_BATTLESTYLE)
#define YPOS_SOUND        ITEM_Y(MENUITEM_SOUND)
#define YPOS_BUTTONMODE   ITEM_Y(MENUITEM_BUTTONMODE)
#define YPOS_FRAMETYPE    ITEM_Y(MENUITEM_FRAMETYPE)
#define YPOS_EXPMODE      ITEM_Y(MENUITEM_EXPMODE)
#define YPOS_ZMOVES       ITEM_Y(MENUITEM_ZMOVES)
#define YPOS_GAMESPEED    ITEM_Y(MENUITEM_GAMESPEED)
#define YPOS_BATTLESPEED  ITEM_Y(MENUITEM_BATTLESPEED)
#define YPOS_WILDMONS     ITEM_Y(MENUITEM_WILDMONS)

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static u8 TextSpeed_ProcessInput(u8 selection);
static void TextSpeed_DrawChoices(u8 selection);
static u8 BattleScene_ProcessInput(u8 selection);
static void BattleScene_DrawChoices(u8 selection);
static u8 BattleStyle_ProcessInput(u8 selection);
static void BattleStyle_DrawChoices(u8 selection);
static u8 Sound_ProcessInput(u8 selection);
static void Sound_DrawChoices(u8 selection);
static u8 FrameType_ProcessInput(u8 selection);
static void FrameType_DrawChoices(u8 selection);
static u8 ButtonMode_ProcessInput(u8 selection);
static void ButtonMode_DrawChoices(u8 selection);
static void DrawHeaderText(void);
static void DrawOptionMenuTexts(void);
static void DrawBgWindowFrames(void);
static u8 Toggle_ProcessInput(u8 selection);
static void ExpMode_DrawChoices(u8 selection);
static void ZMoves_DrawChoices(u8 selection);
static void WildMons_DrawChoices(u8 selection);
static void RedrawVisibleOptions(u8 taskId);
static u8 Cycle_ProcessInput(u8 selection, u8 count);
static void Speed_DrawChoices(u8 selection, u8 y, bool32 battle);

EWRAM_DATA static bool8 sArrowPressed = FALSE;
EWRAM_DATA static u8 sTopItem = 0;

static const u8 gText_Option[]             = _("OPTION");
static const u8 gText_TextSpeedSlow[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SLOW");
static const u8 gText_TextSpeedMid[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MID");
static const u8 gText_TextSpeedFast[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FAST");
static const u8 gText_TextSpeedInstant[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}INSTANT");
static const u8 gText_ArrowLeft[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}<");
static const u8 gText_ArrowRight[]         = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}>");
static const u8 gText_BattleSceneOn[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 gText_BattleSceneOff[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 gText_BattleStyleShift[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SHIFT");
static const u8 gText_BattleStyleSet[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SET");
static const u8 gText_SoundMono[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MONO");
static const u8 gText_SoundStereo[]        = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}STEREO");
static const u8 gText_FrameType[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}TYPE");
static const u8 gText_FrameTypeNumber[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}");
static const u8 gText_ButtonTypeNormal[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NORMAL");
static const u8 gText_ButtonTypeLR[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}LR");
static const u8 gText_ButtonTypeLEqualsA[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}L=A");
static const u8 gText_ExpModeShare[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SHARE");
static const u8 gText_ExpModeAll[]         = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ALL");
static const u8 gText_Speed1x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}1x");
static const u8 gText_Speed2x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}2x");
static const u8 gText_Speed3x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}3x");
static const u8 gText_Speed4x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}4x");

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = COMPOUND_STRING("TEXT SPEED"),
    [MENUITEM_BATTLESCENE] = COMPOUND_STRING("BATTLE SCENE"),
    [MENUITEM_BATTLESTYLE] = COMPOUND_STRING("BATTLE STYLE"),
    [MENUITEM_SOUND]       = COMPOUND_STRING("SOUND"),
    [MENUITEM_BUTTONMODE]  = COMPOUND_STRING("BUTTON MODE"),
    [MENUITEM_FRAMETYPE]   = COMPOUND_STRING("FRAME"),
    [MENUITEM_EXPMODE]     = COMPOUND_STRING("EXP. MODE"),
    [MENUITEM_ZMOVES]      = COMPOUND_STRING("Z-MOVES"),
    [MENUITEM_GAMESPEED]   = COMPOUND_STRING("GAME SPEED"),
    [MENUITEM_BATTLESPEED] = COMPOUND_STRING("BATTLE SPEED"),
    [MENUITEM_WILDMONS]    = COMPOUND_STRING("SEE WILD MONS"),
    [MENUITEM_CANCEL]      = COMPOUND_STRING("CANCEL"),
};

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(WIN_HEADER);
        DrawHeaderText();
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(WIN_OPTIONS);
        DrawOptionMenuTexts();
        gMain.state++;
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);

        gTasks[taskId].tMenuSelection = 0;
        sTopItem = 0;
#if IS_FRLG
        // These options store their flags only in Emerald's save flag table.
        gTasks[taskId].tExpAll = 0;
        gTasks[taskId].tZMovesOff = 0;
        gTasks[taskId].tWildMonsOff = 0;
#else
        gTasks[taskId].tExpAll = FlagGet(FLAG_WALLY_EXP_ALL_MODE);
        gTasks[taskId].tZMovesOff = FlagGet(FLAG_WALLY_Z_MOVES_OFF);
        gTasks[taskId].tWildMonsOff = FlagGet(FLAG_WALLY_OWE_OFF);
#endif
        gTasks[taskId].tGameSpeed = WallyJourney_GetOverworldSpeed() % WALLY_OW_SPEED_COUNT;
        gTasks[taskId].tBattleSpeed = WallyJourney_GetBattleSpeed() % WALLY_BATTLE_SPEED_COUNT;
        gTasks[taskId].tTextSpeed = gSaveBlock2Ptr->optionsTextSpeed;
        gTasks[taskId].tBattleSceneOff = gSaveBlock2Ptr->optionsBattleSceneOff;
        gTasks[taskId].tBattleStyle = gSaveBlock2Ptr->optionsBattleStyle;
        gTasks[taskId].tSound = gSaveBlock2Ptr->optionsSound;
        gTasks[taskId].tButtonMode = gSaveBlock2Ptr->optionsButtonMode;
        gTasks[taskId].tWindowFrameType = gSaveBlock2Ptr->optionsWindowFrameType;

        RedrawVisibleOptions(taskId);
        gMain.state++;
        break;
    }
    case 11:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        if (gTasks[taskId].tMenuSelection == MENUITEM_CANCEL)
            gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tMenuSelection > 0)
            gTasks[taskId].tMenuSelection--;
        else
            gTasks[taskId].tMenuSelection = MENUITEM_CANCEL;
        RedrawVisibleOptions(taskId);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tMenuSelection < MENUITEM_CANCEL)
            gTasks[taskId].tMenuSelection++;
        else
            gTasks[taskId].tMenuSelection = 0;
        RedrawVisibleOptions(taskId);
    }
    else
    {
        u8 previousOption;

        switch (gTasks[taskId].tMenuSelection)
        {
        case MENUITEM_TEXTSPEED:
            previousOption = gTasks[taskId].tTextSpeed;
            gTasks[taskId].tTextSpeed = TextSpeed_ProcessInput(gTasks[taskId].tTextSpeed);

            if (previousOption != gTasks[taskId].tTextSpeed)
                TextSpeed_DrawChoices(gTasks[taskId].tTextSpeed);
            break;
        case MENUITEM_BATTLESCENE:
            previousOption = gTasks[taskId].tBattleSceneOff;
            gTasks[taskId].tBattleSceneOff = BattleScene_ProcessInput(gTasks[taskId].tBattleSceneOff);

            if (previousOption != gTasks[taskId].tBattleSceneOff)
                BattleScene_DrawChoices(gTasks[taskId].tBattleSceneOff);
            break;
        case MENUITEM_BATTLESTYLE:
            previousOption = gTasks[taskId].tBattleStyle;
            gTasks[taskId].tBattleStyle = BattleStyle_ProcessInput(gTasks[taskId].tBattleStyle);

            if (previousOption != gTasks[taskId].tBattleStyle)
                BattleStyle_DrawChoices(gTasks[taskId].tBattleStyle);
            break;
        case MENUITEM_SOUND:
            previousOption = gTasks[taskId].tSound;
            gTasks[taskId].tSound = Sound_ProcessInput(gTasks[taskId].tSound);

            if (previousOption != gTasks[taskId].tSound)
                Sound_DrawChoices(gTasks[taskId].tSound);
            break;
        case MENUITEM_BUTTONMODE:
            previousOption = gTasks[taskId].tButtonMode;
            gTasks[taskId].tButtonMode = ButtonMode_ProcessInput(gTasks[taskId].tButtonMode);

            if (previousOption != gTasks[taskId].tButtonMode)
                ButtonMode_DrawChoices(gTasks[taskId].tButtonMode);
            break;
        case MENUITEM_FRAMETYPE:
            previousOption = gTasks[taskId].tWindowFrameType;
            gTasks[taskId].tWindowFrameType = FrameType_ProcessInput(gTasks[taskId].tWindowFrameType);

            if (previousOption != gTasks[taskId].tWindowFrameType)
                FrameType_DrawChoices(gTasks[taskId].tWindowFrameType);
            break;
        case MENUITEM_EXPMODE:
            previousOption = gTasks[taskId].tExpAll;
            gTasks[taskId].tExpAll = Toggle_ProcessInput(gTasks[taskId].tExpAll);
            if (previousOption != gTasks[taskId].tExpAll)
                ExpMode_DrawChoices(gTasks[taskId].tExpAll);
            break;
        case MENUITEM_ZMOVES:
            previousOption = gTasks[taskId].tZMovesOff;
            gTasks[taskId].tZMovesOff = Toggle_ProcessInput(gTasks[taskId].tZMovesOff);
            if (previousOption != gTasks[taskId].tZMovesOff)
                ZMoves_DrawChoices(gTasks[taskId].tZMovesOff);
            break;
        case MENUITEM_WILDMONS:
            previousOption = gTasks[taskId].tWildMonsOff;
            gTasks[taskId].tWildMonsOff = Toggle_ProcessInput(gTasks[taskId].tWildMonsOff);
            if (previousOption != gTasks[taskId].tWildMonsOff)
                WildMons_DrawChoices(gTasks[taskId].tWildMonsOff);
            break;
        case MENUITEM_GAMESPEED:
            previousOption = gTasks[taskId].tGameSpeed;
            gTasks[taskId].tGameSpeed = Cycle_ProcessInput(gTasks[taskId].tGameSpeed, WALLY_OW_SPEED_COUNT);
            if (previousOption != gTasks[taskId].tGameSpeed)
                Speed_DrawChoices(gTasks[taskId].tGameSpeed, YPOS_GAMESPEED, FALSE);
            break;
        case MENUITEM_BATTLESPEED:
            previousOption = gTasks[taskId].tBattleSpeed;
            gTasks[taskId].tBattleSpeed = Cycle_ProcessInput(gTasks[taskId].tBattleSpeed, WALLY_BATTLE_SPEED_COUNT);
            if (previousOption != gTasks[taskId].tBattleSpeed)
                Speed_DrawChoices(gTasks[taskId].tBattleSpeed, YPOS_BATTLESPEED, TRUE);
            break;
        default:
            return;
        }

        if (sArrowPressed)
        {
            sArrowPressed = FALSE;
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsTextSpeed = gTasks[taskId].tTextSpeed;
    gSaveBlock2Ptr->optionsBattleSceneOff = gTasks[taskId].tBattleSceneOff;
    gSaveBlock2Ptr->optionsBattleStyle = gTasks[taskId].tBattleStyle;
    gSaveBlock2Ptr->optionsSound = gTasks[taskId].tSound;
    gSaveBlock2Ptr->optionsButtonMode = gTasks[taskId].tButtonMode;
    gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].tWindowFrameType;
#if !IS_FRLG
    if (gTasks[taskId].tExpAll)
        FlagSet(FLAG_WALLY_EXP_ALL_MODE);
    else
        FlagClear(FLAG_WALLY_EXP_ALL_MODE);
    WallyJourney_SyncExpItem(); // swap the Exp. Share / EXP. ALL item to match
    if (gTasks[taskId].tZMovesOff)
        FlagSet(FLAG_WALLY_Z_MOVES_OFF);
    else
        FlagClear(FLAG_WALLY_Z_MOVES_OFF);
    if (gTasks[taskId].tWildMonsOff)
        FlagSet(FLAG_WALLY_OWE_OFF);
    else
        FlagClear(FLAG_WALLY_OWE_OFF);
#endif
    WallyJourney_SetSpeeds(gTasks[taskId].tGameSpeed, gTasks[taskId].tBattleSpeed);

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    index -= sTopItem;
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[16];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);

    if (style != 0)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static u8 TextSpeed_ProcessInput(u8 selection)
{
    // Wally's Journey: SLOW / MID / FAST / INSTANT. INSTANT is the in-ROM
    // speed-up; it never touches music, unlike emulator fast-forward.
    if (JOY_NEW(DPAD_RIGHT))
    {
        selection = (selection < OPTIONS_TEXT_SPEED_INSTANT) ? selection + 1 : 0;
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        selection = (selection != 0) ? selection - 1 : OPTIONS_TEXT_SPEED_INSTANT;
        sArrowPressed = TRUE;
    }
    return selection;
}

static void TextSpeed_DrawChoices(u8 selection)
{
    // Four speeds don't fit side by side, so show the current one between arrows.
    static const u8 *const sNames[] = {gText_TextSpeedSlow, gText_TextSpeedMid, gText_TextSpeedFast, gText_TextSpeedInstant};
    if (selection > OPTIONS_TEXT_SPEED_INSTANT)
        selection = OPTIONS_TEXT_SPEED_FAST;
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, YPOS_TEXTSPEED, 96, 16);
    DrawOptionMenuChoice(gText_ArrowLeft, 104, YPOS_TEXTSPEED, 0);
    DrawOptionMenuChoice(sNames[selection], 116, YPOS_TEXTSPEED, 1);
    DrawOptionMenuChoice(gText_ArrowRight, 190, YPOS_TEXTSPEED, 0);
}

static u8 BattleScene_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleScene_DrawChoices(u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleSceneOn, 104, YPOS_BATTLESCENE, styles[0]);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, 198), YPOS_BATTLESCENE, styles[1]);
}

static u8 BattleStyle_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleStyle_DrawChoices(u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleStyleShift, 104, YPOS_BATTLESTYLE, styles[0]);
    DrawOptionMenuChoice(gText_BattleStyleSet, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleStyleSet, 198), YPOS_BATTLESTYLE, styles[1]);
}

static u8 Sound_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        SetPokemonCryStereo(selection);
        sArrowPressed = TRUE;
    }

    return selection;
}

static void Sound_DrawChoices(u8 selection)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_SoundMono, 104, YPOS_SOUND, styles[0]);
    DrawOptionMenuChoice(gText_SoundStereo, GetStringRightAlignXOffset(FONT_NORMAL, gText_SoundStereo, 198), YPOS_SOUND, styles[1]);
}

static u8 FrameType_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection < WINDOW_FRAMES_COUNT - 1)
            selection++;
        else
            selection = 0;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = WINDOW_FRAMES_COUNT - 1;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    return selection;
}

static void FrameType_DrawChoices(u8 selection)
{
    u8 text[16] = {EOS};
    u8 n = selection + 1;
    u16 i;

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i <= 5; i++)
        text[i] = gText_FrameTypeNumber[i];

    // Convert a number to decimal string
    if (n / 10 != 0)
    {
        text[i] = n / 10 + CHAR_0;
        i++;
        text[i] = n % 10 + CHAR_0;
        i++;
    }
    else
    {
        text[i] = n % 10 + CHAR_0;
        i++;
        text[i] = CHAR_SPACER;
        i++;
    }

    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, 104, YPOS_FRAMETYPE, 0);
    DrawOptionMenuChoice(text, 128, YPOS_FRAMETYPE, 1);
}

static u8 ButtonMode_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void ButtonMode_DrawChoices(u8 selection)
{
    s32 widthNormal, widthLR, widthLA, xLR;
    u8 styles[3];

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_ButtonTypeNormal, 104, YPOS_BUTTONMODE, styles[0]);

    widthNormal = GetStringWidth(FONT_NORMAL, gText_ButtonTypeNormal, 0);
    widthLR = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLR, 0);
    widthLA = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLEqualsA, 0);

    widthLR -= 94;
    xLR = (widthNormal - widthLR - widthLA) / 2 + 104;
    DrawOptionMenuChoice(gText_ButtonTypeLR, xLR, YPOS_BUTTONMODE, styles[1]);

    DrawOptionMenuChoice(gText_ButtonTypeLEqualsA, GetStringRightAlignXOffset(FONT_NORMAL, gText_ButtonTypeLEqualsA, 198), YPOS_BUTTONMODE, styles[2]);
}

static void DrawHeaderText(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawOptionMenuTexts(void)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = sTopItem; i < MENUITEM_COUNT && i < sTopItem + VISIBLE_ITEMS; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[i], 8, ITEM_Y(i) + 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}

// Two-choice rows: left/right flips between the two settings.
static u8 Toggle_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }
    return selection;
}

// EXP. MODE: SHARE = the normal held EXP. SHARE item; ALL = the same total EXP
// split across the whole party (see Cmd_getexp).
static void ExpMode_DrawChoices(u8 selection)
{
    DrawOptionMenuChoice(gText_ExpModeShare, 104, YPOS_EXPMODE, selection == 0);
    DrawOptionMenuChoice(gText_ExpModeAll, GetStringRightAlignXOffset(FONT_NORMAL, gText_ExpModeAll, 198), YPOS_EXPMODE, selection == 1);
}

// Z-MOVES: stored as "off" so a new game starts with them ON.
static void ZMoves_DrawChoices(u8 selection)
{
    DrawOptionMenuChoice(gText_BattleSceneOn, 104, YPOS_ZMOVES, selection == 0);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, 198), YPOS_ZMOVES, selection == 1);
}

// SEE WILD MONS: visible wild Pokémon in the overworld. Stored as "off" so a
// new game starts with them ON.
static void WildMons_DrawChoices(u8 selection)
{
    DrawOptionMenuChoice(gText_BattleSceneOn, 104, YPOS_WILDMONS, selection == 0);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, 198), YPOS_WILDMONS, selection == 1);
}

// Scroll so the cursor row is on screen, then redraw every visible row.
static void RedrawVisibleOptions(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 sel = tMenuSelection;

    if (sel < sTopItem)
        sTopItem = sel;
    else if (sel >= sTopItem + VISIBLE_ITEMS)
        sTopItem = sel - VISIBLE_ITEMS + 1;

    DrawOptionMenuTexts();
    if (ITEM_VISIBLE(MENUITEM_TEXTSPEED))   TextSpeed_DrawChoices(tTextSpeed);
    if (ITEM_VISIBLE(MENUITEM_BATTLESCENE)) BattleScene_DrawChoices(tBattleSceneOff);
    if (ITEM_VISIBLE(MENUITEM_BATTLESTYLE)) BattleStyle_DrawChoices(tBattleStyle);
    if (ITEM_VISIBLE(MENUITEM_SOUND))       Sound_DrawChoices(tSound);
    if (ITEM_VISIBLE(MENUITEM_BUTTONMODE))  ButtonMode_DrawChoices(tButtonMode);
    if (ITEM_VISIBLE(MENUITEM_FRAMETYPE))   FrameType_DrawChoices(tWindowFrameType);
    if (ITEM_VISIBLE(MENUITEM_EXPMODE))     ExpMode_DrawChoices(tExpAll);
    if (ITEM_VISIBLE(MENUITEM_ZMOVES))      ZMoves_DrawChoices(tZMovesOff);
    if (ITEM_VISIBLE(MENUITEM_GAMESPEED))   Speed_DrawChoices(tGameSpeed, YPOS_GAMESPEED, FALSE);
    if (ITEM_VISIBLE(MENUITEM_BATTLESPEED)) Speed_DrawChoices(tBattleSpeed, YPOS_BATTLESPEED, TRUE);
    if (ITEM_VISIBLE(MENUITEM_WILDMONS))    WildMons_DrawChoices(tWildMonsOff);
    HighlightOptionMenuItem(sel);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

// Left/right steps through `count` settings, wrapping around.
static u8 Cycle_ProcessInput(u8 selection, u8 count)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        selection = (selection + 1) % count;
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        selection = (selection + count - 1) % count;
        sArrowPressed = TRUE;
    }
    return selection;
}

// GAME SPEED: 1x / 2x / 4x (overworld walking and animations; hold R for 1x).
// BATTLE SPEED: 1x / 2x / 3x / 4x. Neither changes music speed.
static void Speed_DrawChoices(u8 selection, u8 y, bool32 battle)
{
    static const u8 *const sOverworld[] = {gText_Speed1x, gText_Speed2x, gText_Speed4x};
    static const u8 *const sBattle[] = {gText_Speed1x, gText_Speed2x, gText_Speed3x, gText_Speed4x};
    const u8 *name = battle ? sBattle[selection % WALLY_BATTLE_SPEED_COUNT] : sOverworld[selection % WALLY_OW_SPEED_COUNT];
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 16);
    DrawOptionMenuChoice(gText_ArrowLeft, 104, y, 0);
    DrawOptionMenuChoice(name, 140, y, 1);
    DrawOptionMenuChoice(gText_ArrowRight, 190, y, 0);
}

