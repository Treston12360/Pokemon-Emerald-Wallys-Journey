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
#ifdef PORTABLE
#include "platform.h"
#include "string_util.h"
#include "sound.h"
#include "constants/songs.h"
#include "reload_save.h"
#include "save.h"
#include "m4a.h"
#endif

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
#define tNuzlocke data[12]
#define tLevelCap data[13]
#define tClassicGfx data[14] // Wally's Journey: one bit per WJGFX_* feature, set = Classic
#define tQuickCatch data[15] // Wally's Journey: Quick Catch on/off

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
    MENUITEM_NUZLOCKE,
    MENUITEM_LEVELCAP,
    MENUITEM_QUICKCATCH,
    MENUITEM_GFX_FIRST, // Wally's Journey: New / Classic graphics, one row per WJGFX_* feature
    MENUITEM_GFX_LAST = MENUITEM_GFX_FIRST + WJGFX_COUNT - 1,
#ifdef PORTABLE
    // PC version only: stored in the PC settings file, applied immediately.
    MENUITEM_PC_WINDOWSIZE,
    MENUITEM_PC_FULLSCREEN,
    MENUITEM_PC_SHARP,
    MENUITEM_PC_VOLUME,
    MENUITEM_PC_FFSPEED,
    MENUITEM_PC_FFTOGGLE,
    MENUITEM_PC_PAD_A,
    MENUITEM_PC_PAD_B,
    MENUITEM_PC_PAD_START,
    MENUITEM_PC_PAD_SELECT,
    MENUITEM_PC_PAD_L,
    MENUITEM_PC_PAD_R,
    MENUITEM_PC_PAD_FF,
    MENUITEM_PC_WIDESCREEN,
    MENUITEM_PC_SMOOTH,
    MENUITEM_PC_FILTER,
    MENUITEM_PC_LOCKASPECT,
    MENUITEM_PC_BILINEAR,
    MENUITEM_PC_INTERFRAME,
    MENUITEM_PC_LOCKFRAME,
    MENUITEM_PC_RESOLUTION,
    MENUITEM_PC_ZOOM,
    MENUITEM_PC_WORLD3D, // DLX-6
    MENUITEM_PC_LAST = MENUITEM_PC_WORLD3D,
    // Keyboard keys (enum KeyBind order): A opens "Press a key".
    MENUITEM_PC_KEY_FIRST,
    MENUITEM_PC_KEY_LAST = MENUITEM_PC_KEY_FIRST + KEYBIND_COUNT - 1,
    // Rows of the top Settings page: each opens one section.
    MENUITEM_PC_SECTION_GAMEPLAY,
    MENUITEM_PC_SECTION_SPEED,
    MENUITEM_PC_SECTION_CONTROLS,
    MENUITEM_PC_SECTION_KEYBOARD,
    MENUITEM_PC_SECTION_AUDIO,
    MENUITEM_PC_SECTION_VIDEO,
    MENUITEM_PC_RELOAD, // top page: reload the last save (A twice)
#endif
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
#ifdef PORTABLE
// PC: the Settings menu is split into sections. sPcRows lists the rows of the page on screen,
// sPcRowPos[item] is an item's place on that page (PC_ROW_NONE = not on this page).
#define PC_ROW_NONE 0xFF
static u8 sPcRows[32];
static u8 sPcRowCount;
static u8 sPcRowPos[MENUITEM_COUNT];
static u8 sPcPage; // 0 = section list, else MENUITEM_PC_SECTION_* that is open
#define ITEM_Y(item)      ((sPcRowPos[item] - sTopItem) * 16)
#define ITEM_VISIBLE(item) (sPcRowPos[item] != PC_ROW_NONE && sPcRowPos[item] >= sTopItem && sPcRowPos[item] < sTopItem + VISIBLE_ITEMS)
#define SEL_ITEM(taskId)  (sPcRows[gTasks[taskId].tMenuSelection])
#else
#define ITEM_Y(item)      (((item) - sTopItem) * 16)
#define ITEM_VISIBLE(item) ((item) >= sTopItem && (item) < sTopItem + VISIBLE_ITEMS)
#define SEL_ITEM(taskId)  (gTasks[taskId].tMenuSelection)
#endif
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
#define YPOS_NUZLOCKE     ITEM_Y(MENUITEM_NUZLOCKE)
#define YPOS_LEVELCAP     ITEM_Y(MENUITEM_LEVELCAP)
#define YPOS_QUICKCATCH   ITEM_Y(MENUITEM_QUICKCATCH)

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
static void OnOff_DrawChoices(u8 selection, u8 y);
static void Gfx_DrawChoices(u8 classic, u8 y);
static void RedrawVisibleOptions(u8 taskId);
static u8 Cycle_ProcessInput(u8 selection, u8 count);
static void Speed_DrawChoices(u8 selection, u8 y, bool32 battle);
#ifdef PORTABLE
static void PcOption_ProcessInput(u8 item);
static void PcOption_Draw(u8 item);
static bool32 PcMenu_HandleNavigation(u8 taskId);
static void PcMenu_SetPage(u8 page);
static void PcSection_Draw(u8 item);
static void PcReload_Draw(void);
static void Task_PcReloadFadeOut(u8 taskId);
static bool8 sPcReloadConfirm; // "Reload save": the first A asks, the second A reloads
static void PcKey_Draw(u8 item);
#endif

EWRAM_DATA static bool8 sArrowPressed = FALSE;
EWRAM_DATA static u8 sTopItem = 0;

static const u8 gText_Option[]             = _("Option");
static const u8 gText_TextSpeedSlow[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Slow");
static const u8 gText_TextSpeedMid[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Mid");
static const u8 gText_TextSpeedFast[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Fast");
static const u8 gText_TextSpeedInstant[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Instant");
static const u8 gText_ArrowLeft[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}<");
static const u8 gText_ArrowRight[]         = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}>");
static const u8 gText_BattleSceneOn[]      = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}On");
static const u8 gText_BattleSceneOff[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Off");
static const u8 gText_BattleStyleShift[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Shift");
static const u8 gText_BattleStyleSet[]     = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Set");
static const u8 gText_SoundMono[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Mono");
static const u8 gText_SoundStereo[]        = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Stereo");
static const u8 gText_FrameType[]          = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Type");
static const u8 gText_FrameTypeNumber[]    = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}");
static const u8 gText_ButtonTypeNormal[]   = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Normal");
static const u8 gText_ButtonTypeLR[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Lr");
static const u8 gText_ButtonTypeLEqualsA[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}L=A");
static const u8 gText_ExpModeShare[]       = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Share");
static const u8 gText_ExpModeAll[]         = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}All");
static const u8 gText_Speed1x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}1x");
static const u8 gText_Speed2x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}2x");
static const u8 gText_Speed3x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}3x");
static const u8 gText_Speed4x[]            = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}4x");
static const u8 gText_GfxNew[]             = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}New");
static const u8 gText_GfxClassic[]         = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}Classic");

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = COMPOUND_STRING("Text Speed"),
    [MENUITEM_BATTLESCENE] = COMPOUND_STRING("Battle Scene"),
    [MENUITEM_BATTLESTYLE] = COMPOUND_STRING("Battle Style"),
    [MENUITEM_SOUND]       = COMPOUND_STRING("Sound"),
    [MENUITEM_BUTTONMODE]  = COMPOUND_STRING("Button Mode"),
    [MENUITEM_FRAMETYPE]   = COMPOUND_STRING("Frame"),
    [MENUITEM_EXPMODE]     = COMPOUND_STRING("EXP. Mode"),
    [MENUITEM_ZMOVES]      = COMPOUND_STRING("Z-Moves"),
    [MENUITEM_GAMESPEED]   = COMPOUND_STRING("Game Speed"),
    [MENUITEM_BATTLESPEED] = COMPOUND_STRING("Battle Speed"),
    [MENUITEM_WILDMONS]    = COMPOUND_STRING("See Wild Mons"),
    [MENUITEM_NUZLOCKE]    = COMPOUND_STRING("Nuzlocke"),
    [MENUITEM_LEVELCAP]    = COMPOUND_STRING("Level Cap"),
    [MENUITEM_QUICKCATCH]  = COMPOUND_STRING("Quick Catch"),
    [MENUITEM_GFX_FIRST + WJGFX_3D_BACKGROUNDS] = COMPOUND_STRING("Battle BG"),
    [MENUITEM_GFX_FIRST + WJGFX_BATTLE_SCREEN]  = COMPOUND_STRING("Battle Screen"),
    [MENUITEM_GFX_FIRST + WJGFX_SUMMARY]        = COMPOUND_STRING("Summary"),
    [MENUITEM_GFX_FIRST + WJGFX_PARTY]          = COMPOUND_STRING("Party Menu"),
    [MENUITEM_GFX_FIRST + WJGFX_BAG]            = COMPOUND_STRING("Bag"),
    [MENUITEM_GFX_FIRST + WJGFX_PC]             = COMPOUND_STRING("PC Boxes"),
    [MENUITEM_GFX_FIRST + WJGFX_TEXT_BOX]       = COMPOUND_STRING("Text Box"),
    [MENUITEM_GFX_FIRST + WJGFX_START_MENU]     = COMPOUND_STRING("Start Menu"),
    [MENUITEM_GFX_FIRST + WJGFX_MAIN_MENU]      = COMPOUND_STRING("Main Menu"),
    [MENUITEM_GFX_FIRST + WJGFX_POKEDEX]        = COMPOUND_STRING("Pokédex"),
    [MENUITEM_GFX_FIRST + WJGFX_DITTO]          = COMPOUND_STRING("Ditto Faces"),
#ifdef PORTABLE
    [MENUITEM_PC_WINDOWSIZE] = COMPOUND_STRING("Window size"),
    [MENUITEM_PC_FULLSCREEN] = COMPOUND_STRING("Fullscreen"),
    [MENUITEM_PC_SHARP]      = COMPOUND_STRING("Integer scaling"),
    [MENUITEM_PC_VOLUME]     = COMPOUND_STRING("Volume"),
    [MENUITEM_PC_FFSPEED]    = COMPOUND_STRING("Fast forward"),
    [MENUITEM_PC_FFTOGGLE]   = COMPOUND_STRING("Fast fwd button"),
    [MENUITEM_PC_PAD_A]      = COMPOUND_STRING("Pad A"),
    [MENUITEM_PC_PAD_B]      = COMPOUND_STRING("Pad B"),
    [MENUITEM_PC_PAD_START]  = COMPOUND_STRING("Pad Start"),
    [MENUITEM_PC_PAD_SELECT] = COMPOUND_STRING("Pad Select"),
    [MENUITEM_PC_PAD_L]      = COMPOUND_STRING("Pad L"),
    [MENUITEM_PC_PAD_R]      = COMPOUND_STRING("Pad R"),
    [MENUITEM_PC_PAD_FF]     = COMPOUND_STRING("Pad fast fwd"),
    [MENUITEM_PC_WIDESCREEN] = COMPOUND_STRING("Widescreen"),
    [MENUITEM_PC_SMOOTH]     = COMPOUND_STRING("Frame rate"),
    [MENUITEM_PC_FILTER]     = COMPOUND_STRING("Screen filter"),
    [MENUITEM_PC_LOCKASPECT] = COMPOUND_STRING("Lock aspect ratio"),
    [MENUITEM_PC_BILINEAR]   = COMPOUND_STRING("Bilinear filter"),
    [MENUITEM_PC_INTERFRAME] = COMPOUND_STRING("Interframe blend"),
    [MENUITEM_PC_LOCKFRAME]  = COMPOUND_STRING("Lock frame size"),
    [MENUITEM_PC_RESOLUTION] = COMPOUND_STRING("Resolution"),
    [MENUITEM_PC_ZOOM]       = COMPOUND_STRING("Zoom"),
    [MENUITEM_PC_WORLD3D]    = COMPOUND_STRING("3D World"),
    [MENUITEM_PC_SECTION_GAMEPLAY] = COMPOUND_STRING("Gameplay"),
    [MENUITEM_PC_SECTION_SPEED]    = COMPOUND_STRING("Speed & Looks"),
    [MENUITEM_PC_SECTION_CONTROLS] = COMPOUND_STRING("Controller"),
    [MENUITEM_PC_SECTION_KEYBOARD] = COMPOUND_STRING("Keyboard"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_UP]     = COMPOUND_STRING("Key Up"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_DOWN]   = COMPOUND_STRING("Key Down"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_LEFT]   = COMPOUND_STRING("Key Left"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_RIGHT]  = COMPOUND_STRING("Key Right"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_A]      = COMPOUND_STRING("Key A"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_B]      = COMPOUND_STRING("Key B"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_START]  = COMPOUND_STRING("Key Start"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_SELECT] = COMPOUND_STRING("Key Select"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_L]      = COMPOUND_STRING("Key L"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_R]      = COMPOUND_STRING("Key R"),
    [MENUITEM_PC_KEY_FIRST + KEYBIND_FAST_FORWARD] = COMPOUND_STRING("Key fast fwd"),
    [MENUITEM_PC_SECTION_AUDIO]    = COMPOUND_STRING("Audio"),
    [MENUITEM_PC_SECTION_VIDEO]    = COMPOUND_STRING("Video"),
    [MENUITEM_PC_RELOAD]           = COMPOUND_STRING("Reload save"),
#endif
    [MENUITEM_CANCEL]      = COMPOUND_STRING("Cancel"),
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
#ifdef PORTABLE
        PcMenu_SetPage(0);
#endif
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
#ifdef PORTABLE
        PcMenu_SetPage(0);
        DrawHeaderText();
#endif
#if IS_FRLG
        // These options store their flags only in Emerald's save flag table.
        gTasks[taskId].tExpAll = 0;
        gTasks[taskId].tZMovesOff = 0;
        gTasks[taskId].tWildMonsOff = 0;
        gTasks[taskId].tNuzlocke = 0;
        gTasks[taskId].tLevelCap = 0;
        gTasks[taskId].tQuickCatch = 0;
#else
        gTasks[taskId].tExpAll = FlagGet(FLAG_WALLY_EXP_ALL_MODE);
        gTasks[taskId].tZMovesOff = FlagGet(FLAG_WALLY_Z_MOVES_OFF);
        gTasks[taskId].tWildMonsOff = FlagGet(FLAG_WALLY_OWE_OFF);
        gTasks[taskId].tNuzlocke = FlagGet(FLAG_WALLY_NUZLOCKE);
        gTasks[taskId].tLevelCap = FlagGet(FLAG_WALLY_LEVEL_CAP);
        gTasks[taskId].tQuickCatch = FlagGet(FLAG_WALLY_QUICK_CATCH);
#endif
        gTasks[taskId].tClassicGfx = WJ_GetClassicGfxMask();
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
#ifdef PORTABLE
    if (PcMenu_HandleNavigation(taskId))
        return;
#endif
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

        switch (SEL_ITEM(taskId))
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
        case MENUITEM_NUZLOCKE:
            previousOption = gTasks[taskId].tNuzlocke;
            gTasks[taskId].tNuzlocke = Toggle_ProcessInput(gTasks[taskId].tNuzlocke);
            if (previousOption != gTasks[taskId].tNuzlocke)
                OnOff_DrawChoices(gTasks[taskId].tNuzlocke, YPOS_NUZLOCKE);
            break;
        case MENUITEM_LEVELCAP:
            previousOption = gTasks[taskId].tLevelCap;
            gTasks[taskId].tLevelCap = Toggle_ProcessInput(gTasks[taskId].tLevelCap);
            if (previousOption != gTasks[taskId].tLevelCap)
                OnOff_DrawChoices(gTasks[taskId].tLevelCap, YPOS_LEVELCAP);
            break;
        case MENUITEM_QUICKCATCH:
            previousOption = gTasks[taskId].tQuickCatch;
            gTasks[taskId].tQuickCatch = Toggle_ProcessInput(gTasks[taskId].tQuickCatch);
            if (previousOption != gTasks[taskId].tQuickCatch)
                OnOff_DrawChoices(gTasks[taskId].tQuickCatch, YPOS_QUICKCATCH);
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
#ifdef PORTABLE
        case MENUITEM_PC_WINDOWSIZE:
        case MENUITEM_PC_FULLSCREEN:
        case MENUITEM_PC_SHARP:
        case MENUITEM_PC_VOLUME:
        case MENUITEM_PC_FFSPEED:
        case MENUITEM_PC_FFTOGGLE:
        case MENUITEM_PC_PAD_A:
        case MENUITEM_PC_PAD_B:
        case MENUITEM_PC_PAD_START:
        case MENUITEM_PC_PAD_SELECT:
        case MENUITEM_PC_PAD_L:
        case MENUITEM_PC_PAD_R:
        case MENUITEM_PC_PAD_FF:
        case MENUITEM_PC_WIDESCREEN:
        case MENUITEM_PC_SMOOTH:
        case MENUITEM_PC_FILTER:
        case MENUITEM_PC_LOCKASPECT:
        case MENUITEM_PC_BILINEAR:
        case MENUITEM_PC_INTERFRAME:
        case MENUITEM_PC_LOCKFRAME:
        case MENUITEM_PC_RESOLUTION:
        case MENUITEM_PC_ZOOM:
        case MENUITEM_PC_WORLD3D:
            PcOption_ProcessInput(SEL_ITEM(taskId));
            break;
#endif
        default:
            if (SEL_ITEM(taskId) >= MENUITEM_GFX_FIRST && SEL_ITEM(taskId) <= MENUITEM_GFX_LAST)
            {
                u32 bit = SEL_ITEM(taskId) - MENUITEM_GFX_FIRST;
                if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
                {
                    gTasks[taskId].tClassicGfx ^= (1 << bit);
                    sArrowPressed = TRUE;
                    Gfx_DrawChoices((gTasks[taskId].tClassicGfx >> bit) & 1, ITEM_Y(SEL_ITEM(taskId)));
                }
                break;
            }
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
    if (gTasks[taskId].tNuzlocke)
        FlagSet(FLAG_WALLY_NUZLOCKE);
    else
        FlagClear(FLAG_WALLY_NUZLOCKE);
    if (gTasks[taskId].tLevelCap)
        FlagSet(FLAG_WALLY_LEVEL_CAP);
    else
        FlagClear(FLAG_WALLY_LEVEL_CAP);
    if (gTasks[taskId].tQuickCatch)
        FlagSet(FLAG_WALLY_QUICK_CATCH);
    else
        FlagClear(FLAG_WALLY_QUICK_CATCH);
#endif
    WallyJourney_SetSpeeds(gTasks[taskId].tGameSpeed, gTasks[taskId].tBattleSpeed);
    WJ_SetClassicGfxMask((u16)gTasks[taskId].tClassicGfx);

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
#ifdef PORTABLE
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, sPcPage ? sOptionMenuItemsNames[sPcPage] : COMPOUND_STRING("Settings"), 8, 1, TEXT_SKIP_DRAW, NULL);
#else
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
#endif
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawOptionMenuTexts(void)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
#ifdef PORTABLE
    for (i = sTopItem; i < sPcRowCount && i < sTopItem + VISIBLE_ITEMS; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[sPcRows[i]], 8, (i - sTopItem) * 16 + 1, TEXT_SKIP_DRAW, NULL);
#else
    for (i = sTopItem; i < MENUITEM_COUNT && i < sTopItem + VISIBLE_ITEMS; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[i], 8, ITEM_Y(i) + 1, TEXT_SKIP_DRAW, NULL);
#endif
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

// NUZLOCKE / LEVEL CAP (owner request 2026-10-29): stored as "on" so a new game starts with them OFF.
// Value 1 = ON, 0 = OFF; OFF is drawn on the right like the other toggles' second choice.
// Wally's Journey: graphics rows. New = this hack's graphics, Classic = Emerald's.
static void Gfx_DrawChoices(u8 classic, u8 y)
{
    DrawOptionMenuChoice(gText_GfxNew, 104, y, classic == 0);
    DrawOptionMenuChoice(gText_GfxClassic, GetStringRightAlignXOffset(FONT_NORMAL, gText_GfxClassic, 198), y, classic == 1);
}

static void OnOff_DrawChoices(u8 selection, u8 y)
{
    DrawOptionMenuChoice(gText_BattleSceneOn, 104, y, selection == 1);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, 198), y, selection == 0);
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
    if (ITEM_VISIBLE(MENUITEM_NUZLOCKE))    OnOff_DrawChoices(tNuzlocke, YPOS_NUZLOCKE);
    if (ITEM_VISIBLE(MENUITEM_LEVELCAP))    OnOff_DrawChoices(tLevelCap, YPOS_LEVELCAP);
    if (ITEM_VISIBLE(MENUITEM_QUICKCATCH))  OnOff_DrawChoices(tQuickCatch, YPOS_QUICKCATCH);
    {
        u32 i;
        for (i = MENUITEM_GFX_FIRST; i <= MENUITEM_GFX_LAST; i++)
            if (ITEM_VISIBLE(i))
                Gfx_DrawChoices(((u16)tClassicGfx >> (i - MENUITEM_GFX_FIRST)) & 1, ITEM_Y(i));
    }
#ifdef PORTABLE
    {
        u32 i;
        for (i = MENUITEM_PC_WINDOWSIZE; i <= MENUITEM_PC_LAST; i++)
            if (ITEM_VISIBLE(i))
                PcOption_Draw(i);
        for (i = MENUITEM_PC_SECTION_GAMEPLAY; i <= MENUITEM_PC_SECTION_VIDEO; i++)
            if (ITEM_VISIBLE(i))
                PcSection_Draw(i);
        if (ITEM_VISIBLE(MENUITEM_PC_RELOAD))
            PcReload_Draw();
        for (i = MENUITEM_PC_KEY_FIRST; i <= MENUITEM_PC_KEY_LAST; i++)
            if (ITEM_VISIBLE(i))
                PcKey_Draw(i);
    }
#endif
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


#ifdef PORTABLE
// ---- PC version settings rows --------------------------------------------
// Each row maps to a platform setting: its value range and how it is shown.
static const u8 sPcRowSetting[] =
{
    [MENUITEM_PC_WINDOWSIZE - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_WINDOW_SCALE,
    [MENUITEM_PC_FULLSCREEN - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_FULLSCREEN,
    [MENUITEM_PC_SHARP      - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_INTEGER_SCALE,
    [MENUITEM_PC_VOLUME     - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_VOLUME,
    [MENUITEM_PC_FFSPEED    - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_FF_SPEED,
    [MENUITEM_PC_FFTOGGLE   - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_FF_TOGGLE,
    [MENUITEM_PC_PAD_A      - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_A,
    [MENUITEM_PC_PAD_B      - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_B,
    [MENUITEM_PC_PAD_START  - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_START,
    [MENUITEM_PC_PAD_SELECT - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_SELECT,
    [MENUITEM_PC_PAD_L      - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_L,
    [MENUITEM_PC_PAD_R      - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_R,
    [MENUITEM_PC_PAD_FF     - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_PAD_FF,
    [MENUITEM_PC_WIDESCREEN - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_WIDESCREEN,
    [MENUITEM_PC_SMOOTH     - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_SMOOTH,
    [MENUITEM_PC_FILTER     - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_FILTER,
    [MENUITEM_PC_LOCKASPECT - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_LOCK_ASPECT,
    [MENUITEM_PC_BILINEAR   - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_SMOOTHING,
    [MENUITEM_PC_INTERFRAME - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_INTERFRAME,
    [MENUITEM_PC_LOCKFRAME  - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_LOCK_FRAME,
    [MENUITEM_PC_RESOLUTION - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_RESOLUTION,
    [MENUITEM_PC_ZOOM       - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_ZOOM,
    [MENUITEM_PC_WORLD3D    - MENUITEM_PC_WINDOWSIZE] = PLATFORM_SETTING_WORLD3D,
};
#define PAD_MAX (PAD_BUTTON_COUNT - 1)
static const u8 sPcRowMin[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0};
static const u8 sPcRowMax[] = {12, 1, 1, 10, FF_SPEED_COUNT - 1, 1, PAD_MAX, PAD_MAX, PAD_MAX, PAD_MAX, PAD_MAX, PAD_MAX, PAD_MAX, 1, FRAME_RATE_COUNT - 1, 2,
                               1, 1, 1, 1, 4, 7, 1}; // lock aspect, bilinear, interframe, lock frame, resolution, zoom, 3D

static void PcOption_ProcessInput(u8 item)
{
    u32 row = item - MENUITEM_PC_WINDOWSIZE;
    s32 value = Platform_GetSetting(sPcRowSetting[row]);
    s32 old = value;

    // Volume stops at 0 and 10; the other rows (fast forward 20x -> 2x too, owner request) wrap around.
    bool32 wrap = (item != MENUITEM_PC_VOLUME);
    if (JOY_REPEAT(DPAD_RIGHT))
        value = (value < sPcRowMax[row]) ? value + 1 : (wrap ? sPcRowMin[row] : value);
    else if (JOY_REPEAT(DPAD_LEFT))
        value = (value > sPcRowMin[row]) ? value - 1 : (wrap ? sPcRowMax[row] : value);

    if (value != old)
    {
        Platform_SetSetting(sPcRowSetting[row], value);
        PcOption_Draw(item);
        sArrowPressed = TRUE;
    }
}

static void PcOption_Draw(u8 item)
{
    static const u8 sPadNames[PAD_BUTTON_COUNT][6] = {
        [PAD_BUTTON_A] = _("A"), [PAD_BUTTON_B] = _("B"), [PAD_BUTTON_X] = _("X"), [PAD_BUTTON_Y] = _("Y"),
        [PAD_BUTTON_LB] = _("LB"), [PAD_BUTTON_RB] = _("RB"), [PAD_BUTTON_LT] = _("LT"), [PAD_BUTTON_RT] = _("RT"),
        [PAD_BUTTON_BACK] = _("Back"), [PAD_BUTTON_START] = _("Start"),
        [PAD_BUTTON_LSTICK] = _("LS"), [PAD_BUTTON_RSTICK] = _("RS"), [PAD_BUTTON_NONE] = _("None"),
    };
    u32 row = item - MENUITEM_PC_WINDOWSIZE;
    u32 value = Platform_GetSetting(sPcRowSetting[row]);
    u8 text[16];
    u8 *end;
    u8 y = ITEM_Y(item);

    // Same green colour prefix as the other choices.
    memcpy(text, gText_Speed1x, 6);
    end = text + 6;
    switch (item)
    {
    case MENUITEM_PC_WINDOWSIZE:
        end = ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 2);
        *end++ = CHAR_x;
        *end = EOS;
        break;
    case MENUITEM_PC_FULLSCREEN:
    case MENUITEM_PC_SHARP:
    case MENUITEM_PC_WIDESCREEN:
    case MENUITEM_PC_LOCKASPECT:
    case MENUITEM_PC_BILINEAR:
    case MENUITEM_PC_INTERFRAME:
    case MENUITEM_PC_LOCKFRAME:
        StringCopy(end, (value ? gText_BattleSceneOn : gText_BattleSceneOff) + 6);
        break;
    case MENUITEM_PC_VOLUME:
        ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 2);
        break;
    case MENUITEM_PC_FFSPEED: // 2x, 4x ... 20x
        end = ConvertIntToDecimalStringN(end, 2 * (value % FF_SPEED_COUNT + 1), STR_CONV_MODE_LEFT_ALIGN, 2);
        *end++ = CHAR_x;
        *end = EOS;
        break;
    case MENUITEM_PC_FFTOGGLE:
        StringCopy(end, value ? COMPOUND_STRING("Toggle") : COMPOUND_STRING("Hold"));
        break;
    case MENUITEM_PC_SMOOTH:
        // Frames drawn per second; above 60 the extra ones are smooth in-between frames.
        end = ConvertIntToDecimalStringN(end, gPlatformFrameRates[value % FRAME_RATE_COUNT], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringCopy(end, COMPOUND_STRING(" fps"));
        break;
    case MENUITEM_PC_RESOLUTION:
    {
        static const u8 sResNames[][8] = {_("Monitor"), _("720p"), _("1080p"), _("1440p"), _("4K")};
        StringCopy(end, sResNames[value % 5]);
        break;
    }
    case MENUITEM_PC_WORLD3D:
        if (!Platform_Is3DAvailable())
            StringCopy(end, COMPOUND_STRING("N/A")); // no 3D data folder or no OpenGL
        else
            StringCopy(end, (value ? gText_BattleSceneOn : gText_BattleSceneOff) + 6);
        break;
    case MENUITEM_PC_ZOOM: // shown 1 ... 8
        ConvertIntToDecimalStringN(end, value + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        break;
    case MENUITEM_PC_FILTER:
        StringCopy(end, value == 1 ? COMPOUND_STRING("Scanlines") : value == 2 ? COMPOUND_STRING("LCD") : COMPOUND_STRING("Off"));
        break;
    default: // controller buttons
        StringCopy(end, sPadNames[value % PAD_BUTTON_COUNT]);
        break;
    }

    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 16);
    DrawOptionMenuChoice(gText_ArrowLeft, 104, y, 0);
    DrawOptionMenuChoice(text, 140, y, 1);
    DrawOptionMenuChoice(gText_ArrowRight, 190, y, 0);
}
// ---- PC Settings sections ---------------------------------------------------
static const u8 sPcSectionGameplay[] = {MENUITEM_TEXTSPEED, MENUITEM_BATTLESCENE, MENUITEM_BATTLESTYLE, MENUITEM_EXPMODE,
    MENUITEM_ZMOVES, MENUITEM_WILDMONS, MENUITEM_NUZLOCKE, MENUITEM_LEVELCAP, MENUITEM_QUICKCATCH};
static const u8 sPcSectionControls[] = {MENUITEM_BUTTONMODE, MENUITEM_PC_PAD_A, MENUITEM_PC_PAD_B, MENUITEM_PC_PAD_START,
    MENUITEM_PC_PAD_SELECT, MENUITEM_PC_PAD_L, MENUITEM_PC_PAD_R, MENUITEM_PC_PAD_FF};
static const u8 sPcSectionAudio[] = {MENUITEM_SOUND, MENUITEM_PC_VOLUME};
static const u8 sPcSectionVideo[] = {MENUITEM_PC_WINDOWSIZE, MENUITEM_PC_FULLSCREEN, MENUITEM_PC_RESOLUTION, MENUITEM_PC_ZOOM,
    MENUITEM_PC_LOCKASPECT, MENUITEM_PC_SHARP, MENUITEM_PC_BILINEAR, MENUITEM_PC_INTERFRAME, MENUITEM_PC_LOCKFRAME,
    MENUITEM_PC_WIDESCREEN, MENUITEM_PC_SMOOTH, MENUITEM_PC_FILTER, MENUITEM_PC_WORLD3D};

static void PcMenu_AddRow(u8 item)
{
    sPcRowPos[item] = sPcRowCount;
    sPcRows[sPcRowCount++] = item;
}

static void PcMenu_AddRows(const u8 *items, u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        PcMenu_AddRow(items[i]);
}

static void PcMenu_SetPage(u8 page)
{
    u32 i;
    sPcPage = page;
    sPcRowCount = 0;
    for (i = 0; i < MENUITEM_COUNT; i++)
        sPcRowPos[i] = PC_ROW_NONE;
    switch (page)
    {
    case 0:
        for (i = MENUITEM_PC_SECTION_GAMEPLAY; i <= MENUITEM_PC_SECTION_VIDEO; i++)
            PcMenu_AddRow(i);
        PcMenu_AddRow(MENUITEM_PC_RELOAD);
        break;
    case MENUITEM_PC_SECTION_GAMEPLAY:
        PcMenu_AddRows(sPcSectionGameplay, ARRAY_COUNT(sPcSectionGameplay));
        break;
    case MENUITEM_PC_SECTION_SPEED:
        PcMenu_AddRow(MENUITEM_GAMESPEED);
        PcMenu_AddRow(MENUITEM_BATTLESPEED);
        PcMenu_AddRow(MENUITEM_PC_FFSPEED);
        PcMenu_AddRow(MENUITEM_PC_FFTOGGLE);
        PcMenu_AddRow(MENUITEM_FRAMETYPE);
        for (i = MENUITEM_GFX_FIRST; i <= MENUITEM_GFX_LAST; i++)
            PcMenu_AddRow(i);
        break;
    case MENUITEM_PC_SECTION_CONTROLS:
        PcMenu_AddRows(sPcSectionControls, ARRAY_COUNT(sPcSectionControls));
        break;
    case MENUITEM_PC_SECTION_KEYBOARD:
        for (i = MENUITEM_PC_KEY_FIRST; i <= MENUITEM_PC_KEY_LAST; i++)
            PcMenu_AddRow(i);
        break;
    case MENUITEM_PC_SECTION_AUDIO:
        PcMenu_AddRows(sPcSectionAudio, ARRAY_COUNT(sPcSectionAudio));
        break;
    case MENUITEM_PC_SECTION_VIDEO:
        PcMenu_AddRows(sPcSectionVideo, ARRAY_COUNT(sPcSectionVideo));
        break;
    }
    PcMenu_AddRow(MENUITEM_CANCEL);
}

static void PcMenu_OpenPage(u8 taskId, u8 page, u8 selection)
{
    PcMenu_SetPage(page);
    gTasks[taskId].tMenuSelection = selection;
    sTopItem = 0;
    DrawHeaderText();
    RedrawVisibleOptions(taskId);
}

// A opens a section (or leaves with Cancel), B goes back to the section list (or leaves),
// up/down move within the page. Returns TRUE when the input was used here.
static bool32 PcMenu_HandleNavigation(u8 taskId)
{
    static bool8 sWaitingForKey;
    u8 sel = gTasks[taskId].tMenuSelection;
    u8 item = sPcRows[sel];

    // "Press a key": nothing else happens until a key is pressed (Esc cancels).
    if (Platform_IsCapturingKey())
        return TRUE;
    if (sWaitingForKey)
    {
        sWaitingForKey = FALSE;
        PlaySE(SE_SELECT);
        RedrawVisibleOptions(taskId); // a key taken from another button swaps, so redraw all rows
        return TRUE;
    }

    if (item == MENUITEM_PC_RELOAD && JOY_NEW(A_BUTTON))
    {
        if (gSaveFileStatus != SAVE_STATUS_OK)
        {
            PlaySE(SE_FAILURE);
        }
        else if (!sPcReloadConfirm)
        {
            PlaySE(SE_SELECT);
            sPcReloadConfirm = TRUE;
            PcReload_Draw();
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
        else
        {
            // Nothing is saved: settings changed in this visit and progress since the last save are dropped.
            sPcReloadConfirm = FALSE;
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_PcReloadFadeOut;
        }
        return TRUE;
    }
    if (sPcReloadConfirm && (JOY_NEW(B_BUTTON) || JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN)))
    {
        // Any other button cancels the question.
        sPcReloadConfirm = FALSE;
        PcReload_Draw();
        CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        if (JOY_NEW(B_BUTTON))
            return TRUE;
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (item >= MENUITEM_PC_KEY_FIRST && item <= MENUITEM_PC_KEY_LAST)
        {
            PlaySE(SE_SELECT);
            sWaitingForKey = TRUE;
            Platform_StartKeyCapture(item - MENUITEM_PC_KEY_FIRST);
            PcKey_Draw(item);
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
            return TRUE;
        }
        if (item >= MENUITEM_PC_SECTION_GAMEPLAY && item <= MENUITEM_PC_SECTION_VIDEO)
        {
            PlaySE(SE_SELECT);
            PcMenu_OpenPage(taskId, item, 0);
        }
        else if (item == MENUITEM_CANCEL)
        {
            if (sPcPage != 0)
                PcMenu_OpenPage(taskId, 0, sPcPage - MENUITEM_PC_SECTION_GAMEPLAY);
            else
                gTasks[taskId].func = Task_OptionMenuSave;
        }
        return TRUE;
    }
    if (JOY_NEW(B_BUTTON))
    {
        if (sPcPage != 0)
            PcMenu_OpenPage(taskId, 0, sPcPage - MENUITEM_PC_SECTION_GAMEPLAY);
        else
            gTasks[taskId].func = Task_OptionMenuSave;
        return TRUE;
    }
    if (JOY_NEW(DPAD_UP))
    {
        gTasks[taskId].tMenuSelection = (sel > 0) ? sel - 1 : sPcRowCount - 1;
        RedrawVisibleOptions(taskId);
        return TRUE;
    }
    if (JOY_NEW(DPAD_DOWN))
    {
        gTasks[taskId].tMenuSelection = (sel + 1 < sPcRowCount) ? sel + 1 : 0;
        RedrawVisibleOptions(taskId);
        return TRUE;
    }
    return FALSE;
}

// Section list rows: a short note of what is inside, and an arrow.
static void PcSection_Draw(u8 item)
{
    static const u8 *const sNotes[] = {
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Battle, rules"),
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Fast fwd, art"),
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Pad buttons"),
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Keys"),
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Sound, volume"),
        COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Window, size"),
    };
    u8 y = ITEM_Y(item);
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 16);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sNotes[item - MENUITEM_PC_SECTION_GAMEPLAY], 104, y + 1, TEXT_SKIP_DRAW, NULL);
    DrawOptionMenuChoice(gText_ArrowRight, 190, y, 0);
}
// "Reload save" row: what it does, or the question after the first A.
static void PcReload_Draw(void)
{
    u8 y = ITEM_Y(MENUITEM_PC_RELOAD);
    const u8 *note;

    if (gSaveFileStatus != SAVE_STATUS_OK)
        note = COMPOUND_STRING("{COLOR DARK_GRAY}{SHADOW LIGHT_GRAY}No save yet");
    else if (sPcReloadConfirm)
        note = COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}Sure? Press A");
    else
        note = COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}Last save");
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 16);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, note, 104, y + 1, TEXT_SKIP_DRAW, NULL);
}

// After the fade: throw away everything since the last save and continue from it,
// the same way the game recovers from a link error (ReloadSave).
static void Task_PcReloadFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        m4aMPlayAllStop();
        gMain.state = 0;
        ReloadSave();
    }
}

// Keyboard rows: the key's name, or "Press a key" while waiting for one.
static void PcKey_Draw(u8 item)
{
    u8 text[24];
    u8 y = ITEM_Y(item);

    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 104, y, 96, 16);
    if (Platform_IsCapturingKey() && sPcRows[gTasks[FindTaskIdByFunc(Task_OptionMenuProcessInput)].tMenuSelection] == item)
    {
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL,
            COMPOUND_STRING("{COLOR RED}{SHADOW LIGHT_RED}Press a key"), 104, y + 1, TEXT_SKIP_DRAW, NULL);
        return;
    }
    // Same colour prefix as the other choices, then the key name.
    memcpy(text, gText_Speed1x, 6);
    text[2] = TEXT_COLOR_RED;
    text[5] = TEXT_COLOR_LIGHT_RED;
    Platform_GetKeyBindName(item - MENUITEM_PC_KEY_FIRST, text + 6, sizeof(text) - 6);
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, text, 104, y + 1, TEXT_SKIP_DRAW, NULL);
}
#endif
