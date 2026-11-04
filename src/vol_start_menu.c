#include "option_menu.h"
#include "vol_start_menu.h"
#include "global.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_object_lock.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_weather.h"
#include "field_screen_effect.h"
#include "frontier_pass.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item_menu.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "malloc.h"
#include "map_name_popup.h"
#include "menu.h"
#include "new_game.h"
#include "option_menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokenav.h"
#include "safari_zone.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "sprite.h"
#include "sound.h"
#include "start_menu.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_card.h"
#include "window.h"
#include "union_room.h"
#include "constants/battle_frontier.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "rtc.h"
#include "event_object_movement.h"
#include "gba/isagbprint.h"
#include "dexnav.h"
#include "wild_encounter.h"

/* CONFIGS */
#define VOL_START_UPDATE_CLOCK_DISPLAY    TRUE
#define VOL_START_SHORTENED_NAME          FALSE
#define VOL_START_24_HOUR_MODE            TRUE

/* CALLBACKS */
static void SpriteCB_IconPoketch(struct Sprite* sprite);
static void SpriteCB_IconPokedex(struct Sprite* sprite);
static void SpriteCB_IconParty(struct Sprite* sprite);
static void SpriteCB_IconBag(struct Sprite* sprite);
static void SpriteCB_IconTrainerCard(struct Sprite* sprite);
static void SpriteCB_IconSave(struct Sprite* sprite);
static void SpriteCB_IconOptions(struct Sprite* sprite);
static void SpriteCB_IconFlag(struct Sprite* sprite);

/* TASKs */
static void Task_Vol_StartMenu_HandleMainInput(u8 taskId);
static void Vol_StartMenu_ExitAndClearTilemap(void);
static void Task_Vol_StartMenu_WaitSaveGame(u8 taskId);

/* UNLOCKED FUNC */
static bool32 Vol_UnlockedFunc_Unlocked(void);
static bool32 Vol_UnlockedFunc_Pokedex(void);
static bool32 Vol_UnlockedFunc_Pokemon(void);
static bool32 Vol_UnlockedFunc_PokeNav(void);
static bool32 Vol_UnlockedFunc_Save(void);
static bool32 Vol_UnlockedFunc_SafariFlag(void);

/* SELECTED FUNC */
static void Vol_SelectedFunc_Pokedex(void);
static void Vol_SelectedFunc_Pokemon(void);
static void Vol_SelectedFunc_Bag(void);
static void Vol_SelectedFunc_PokeNav(void);
static void Vol_SelectedFunc_Trainer(void);
static void Vol_SelectedFunc_Save(void);
static void Vol_SelectedFunc_Settings(void);
static void Vol_SelectedFunc_SafariFlag(void);

/* OTHER FUNCTIONS */
static void Vol_StartMenu_LoadSprites(void);
static void Vol_StartMenu_CreateAllSprites(void);
static void Vol_StartMenu_LoadBgGfx(void);
static void Vol_StartMenu_ShowTimeWindow(void);
static void Vol_StartMenu_PrintClockDisplay(void);
static void Vol_StartMenu_UpdateMenuName(void);

/* ENUMs */
enum VolMenuItems
{
    VOL_START_MENU_FLAG,
    VOL_START_MENU_POKEDEX,
    VOL_START_MENU_PARTY,
    VOL_START_MENU_BAG,
    VOL_START_MENU_POKENAV,
    VOL_START_MENU_TRAINER_CARD,
    VOL_START_MENU_SAVE,
    VOL_START_MENU_OPTIONS,
    VOL_START_MENU_COUNT,
};
#define VOL_START_MENU_FIRST_OPTION VOL_START_MENU_COUNT - VOL_START_MENU_COUNT

enum VolSmallOptions
{
    VOL_START_MENU_OPTION_1,
    VOL_START_MENU_OPTION_2,
    VOL_START_MENU_OPTION_3,
    VOL_START_MENU_OPTION_4,
    VOL_START_MENU_OPTION_5,
    VOL_START_MENU_OPTION_6,
    VOL_START_MENU_OPTION_7,
    VOL_START_MENU_OPTION_COUNT,
};

enum VolSpriteAnims
{
    SPRITE_INACTIVE,
    SPRITE_ACTIVE,
};

/* STRUCTs */
struct VolMenuOptions
{
    const u8 *menuName;
    bool32 (*unlockedFunc)(void);
    void (*selectedFunc)(void);
    const struct SpriteTemplate *iconTemplate;
    s32 yOffset;
};

struct Vol_StartMenu
{
    MainCallback savedCallback;
    bool32 isLoading;
    bool32 spriteFlag; // some bool32 holding values for controlling the sprite anims and lifetime
    enum VolMenuItems menuSmallOptions[VOL_START_MENU_OPTION_COUNT];
    u32 menuSmallSpriteId[VOL_START_MENU_OPTION_COUNT];
    u32 windowIdClock;
    u32 windowIdMenuName;
    u32 windowIdSafariBalls;
    u32 windowIdSaveInfo;
    u32 windowIdDexChoice;
};

static EWRAM_DATA struct Vol_StartMenu *sVol_StartMenu = NULL;
static EWRAM_DATA enum VolMenuItems menuSelected; // Separate memory allocation so it persist between destroying of menu.

// --BG-GFX--
static const u32 sStartMenuTiles[] = INCBIN_U32("graphics/vol_start_menu/bg.4bpp.lz");
static const u32 sStartMenuTilemap[] = INCBIN_U32("graphics/vol_start_menu/bg.bin.lz");
static const u32 sStartMenuTilemapSafari[] = INCBIN_U32("graphics/vol_start_menu/bg_safari.bin.lz");
static const u16 sStartMenuPalette[] = INCBIN_U16("graphics/vol_start_menu/bg.gbapal");

//--SPRITE-GFX--
#define TAG_ICON_GFX 1234
#define TAG_ICON_PAL 0x4654 | BLEND_IMMUNE_FLAG
#define ICON_COORD_X 224
#define ICON_COORD_Y_TOP 17
#define ICON_HEIGHT 32
#define ICON_GAP 3

static const u32 sIconGfx[] = INCBIN_U32("graphics/vol_start_menu/icons.4bpp.lz");
static const u16 sIconPal[] = INCBIN_U16("graphics/vol_start_menu/icons.gbapal");

static const struct WindowTemplate sSaveInfoWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 14,
    .height = 10,
    .paletteNum = 15,
    .baseBlock = 8
};

static const struct WindowTemplate sWindowTemplate_StartClock = {
  .bg = 0, 
  .tilemapLeft = 2, 
  .tilemapTop = 17, 
  .width = 12,
  .height = 2, 
  .paletteNum = 15,
  .baseBlock = 0x30
};

static const struct WindowTemplate sWindowTemplate_MenuName = {
    .bg = 0, 
    .tilemapLeft = 16, 
    .tilemapTop = 17, 
    .width = 7, 
    .height = 2, 
    .paletteNum = 15,
    .baseBlock = 0x30 + (12*2)
};

static const struct WindowTemplate sWindowTemplate_SafariBalls = {
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 1,
    .width = 7,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = (0x30 + (12*2)) + (7*2)
};

static const struct SpritePalette sSpritePal_Icon[] =
{
    {sIconPal, TAG_ICON_PAL},
    {NULL},
};

static const struct CompressedSpriteSheet sSpriteSheet_Icon[] = 
{
    {sIconGfx, 32*512/2 , TAG_ICON_GFX},
    {NULL},
};

static const struct OamData gOamIcon = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .objMode = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
};

static const union AnimCmd gAnimCmdPoketch_NotSelected[] = {
    ANIMCMD_FRAME(112, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdPoketch_Selected[] = {
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconPoketchAnim[] = {
    gAnimCmdPoketch_NotSelected,
    gAnimCmdPoketch_Selected,
};

static const union AnimCmd gAnimCmdPokedex_NotSelected[] = {
    ANIMCMD_FRAME(128, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdPokedex_Selected[] = {
    ANIMCMD_FRAME(16, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconPokedexAnim[] = {
    gAnimCmdPokedex_NotSelected,
    gAnimCmdPokedex_Selected,
};

static const union AnimCmd gAnimCmdParty_NotSelected[] = {
    ANIMCMD_FRAME(144, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdParty_Selected[] = {
    ANIMCMD_FRAME(32, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconPartyAnim[] = {
    gAnimCmdParty_NotSelected,
    gAnimCmdParty_Selected,
};

static const union AnimCmd gAnimCmdBag_NotSelected[] = {
    ANIMCMD_FRAME(160, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdBag_Selected[] = {
    ANIMCMD_FRAME(48, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconBagAnim[] = {
    gAnimCmdBag_NotSelected,
    gAnimCmdBag_Selected,
};

static const union AnimCmd gAnimCmdTrainerCard_NotSelected[] = {
    ANIMCMD_FRAME(176, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdTrainerCard_Selected[] = {
    ANIMCMD_FRAME(64, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconTrainerCardAnim[] = {
    gAnimCmdTrainerCard_NotSelected,
    gAnimCmdTrainerCard_Selected,
};

static const union AnimCmd gAnimCmdSave_NotSelected[] = {
    ANIMCMD_FRAME(192, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdSave_Selected[] = {
    ANIMCMD_FRAME(80, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconSaveAnim[] = {
    gAnimCmdSave_NotSelected,
    gAnimCmdSave_Selected,
};

static const union AnimCmd gAnimCmdOptions_NotSelected[] = {
    ANIMCMD_FRAME(208, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdOptions_Selected[] = {
    ANIMCMD_FRAME(96, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconOptionsAnim[] = {
    gAnimCmdOptions_NotSelected,
    gAnimCmdOptions_Selected,
};

static const union AnimCmd gAnimCmdFlag_NotSelected[] = {
    ANIMCMD_FRAME(240, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd gAnimCmdFlag_Selected[] = {
    ANIMCMD_FRAME(224, 0),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const gIconFlagAnim[] = {
    gAnimCmdFlag_NotSelected,
    gAnimCmdFlag_Selected,
};

static const union AffineAnimCmd sAffineAnimIcon_NoAnim[] =
{
    AFFINEANIMCMD_FRAME(0,0, 0, 60),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAffineAnimIcon_Anim[] =
{
    AFFINEANIMCMD_FRAME(20, 20, 0, 5),    // Scale big
    AFFINEANIMCMD_FRAME(-10, -10, 0, 10), // Scale smol
    AFFINEANIMCMD_FRAME(0, 0, 1, 4),      // Begin rotating

    AFFINEANIMCMD_FRAME(0, 0, -1, 4),     // Loop starts from here ; Rotate/Tilt left 
    AFFINEANIMCMD_FRAME(0, 0, 0, 2),
    AFFINEANIMCMD_FRAME(0, 0, -1, 4),
    AFFINEANIMCMD_FRAME(0, 0, 0, 2),
    AFFINEANIMCMD_FRAME(0, 0, -1, 4),

    AFFINEANIMCMD_FRAME(0, 0, 1, 4),      // Rotate/Tilt Right
    AFFINEANIMCMD_FRAME(0, 0, 0, 2),
    AFFINEANIMCMD_FRAME(0, 0, 1, 4),
    AFFINEANIMCMD_FRAME(0, 0, 0, 2),
    AFFINEANIMCMD_FRAME(0, 0, 1, 4),

    AFFINEANIMCMD_JUMP(3),
};

static const union AffineAnimCmd *const sAffineAnimsIcon[] =
{   
    sAffineAnimIcon_NoAnim,
    sAffineAnimIcon_Anim,
};

static const struct SpriteTemplate gSpriteIconPoketch = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconPoketchAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconPoketch,
};

static const struct SpriteTemplate gSpriteIconPokedex = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconPokedexAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconPokedex,
};

static const struct SpriteTemplate gSpriteIconParty = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconPartyAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconParty,
};

static const struct SpriteTemplate gSpriteIconBag = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconBagAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconBag,
};

static const struct SpriteTemplate gSpriteIconTrainerCard = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconTrainerCardAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconTrainerCard,
};

static const struct SpriteTemplate gSpriteIconSave = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconSaveAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconSave,
};

static const struct SpriteTemplate gSpriteIconOptions = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconOptionsAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconOptions,
};

static const struct SpriteTemplate gSpriteIconFlag = {
    .tileTag = TAG_ICON_GFX,
    .paletteTag = TAG_ICON_PAL,
    .oam = &gOamIcon,
    .anims = gIconFlagAnim,
    .images = NULL,
    .affineAnims = sAffineAnimsIcon,
    .callback = SpriteCB_IconFlag,
};

static void SpriteCB_IconPoketch(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_POKENAV && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_POKENAV)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    }
}

static void SpriteCB_IconPokedex(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_POKEDEX && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_POKEDEX)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    }
}

static void SpriteCB_IconParty(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_PARTY && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_PARTY)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    }
}

static void SpriteCB_IconBag(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_BAG && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_BAG)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    } 
}

static void SpriteCB_IconTrainerCard(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_TRAINER_CARD && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_TRAINER_CARD)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    } 
}

static void SpriteCB_IconSave(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_SAVE && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_SAVE)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    } 
}

static void SpriteCB_IconOptions(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_OPTIONS && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_OPTIONS)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    } 
}

static void SpriteCB_IconFlag(struct Sprite* sprite)
{
    if (menuSelected == VOL_START_MENU_FLAG && sVol_StartMenu->spriteFlag == FALSE)
    {
        sVol_StartMenu->spriteFlag = TRUE;
        StartSpriteAnim(sprite, SPRITE_ACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_ACTIVE);
    }
    else if (menuSelected != VOL_START_MENU_FLAG)
    {
        StartSpriteAnim(sprite, SPRITE_INACTIVE);
        StartSpriteAffineAnim(sprite, SPRITE_INACTIVE);
    } 
}

static struct VolMenuOptions sVolOptions[VOL_START_MENU_COUNT] =
{
    [VOL_START_MENU_POKEDEX] =
    {
        .menuName = COMPOUND_STRING("Pokédex"),
        .unlockedFunc = Vol_UnlockedFunc_Pokedex,
        .selectedFunc = Vol_SelectedFunc_Pokedex,
        .iconTemplate = &gSpriteIconPokedex,
        .yOffset = 7,
    },
    [VOL_START_MENU_PARTY] =
    {
        .menuName = COMPOUND_STRING("Party"),
        .unlockedFunc = Vol_UnlockedFunc_Pokemon,
        .selectedFunc = Vol_SelectedFunc_Pokemon,
        .iconTemplate = &gSpriteIconParty,
        .yOffset = 7,
    },
    [VOL_START_MENU_BAG] =
    {
        .menuName = COMPOUND_STRING("Bag"),
        .unlockedFunc = Vol_UnlockedFunc_Unlocked,
        .selectedFunc = Vol_SelectedFunc_Bag,
        .iconTemplate = &gSpriteIconBag,
        .yOffset = 4,
    },
    [VOL_START_MENU_POKENAV] =
    {
        .menuName = COMPOUND_STRING("PokéNav"),
        .unlockedFunc = Vol_UnlockedFunc_PokeNav,
        .selectedFunc = Vol_SelectedFunc_PokeNav,
        .iconTemplate = &gSpriteIconPoketch,
        .yOffset = 4,
    },
    [VOL_START_MENU_TRAINER_CARD] =
    {
        .menuName = COMPOUND_STRING("Trainer"),
        .unlockedFunc = Vol_UnlockedFunc_Unlocked,
        .selectedFunc = Vol_SelectedFunc_Trainer,
        .iconTemplate = &gSpriteIconTrainerCard,
        .yOffset = 8,
    },
    [VOL_START_MENU_SAVE] =
    {
        .menuName = COMPOUND_STRING("Save"),
        .unlockedFunc = Vol_UnlockedFunc_Save,
        .selectedFunc = Vol_SelectedFunc_Save,
        .iconTemplate = &gSpriteIconSave,
        .yOffset = 6,
    },
    [VOL_START_MENU_OPTIONS] =
    {
        .menuName = COMPOUND_STRING("Settings"),
        .unlockedFunc = Vol_UnlockedFunc_Unlocked,
        .selectedFunc = Vol_SelectedFunc_Settings,
        .iconTemplate = &gSpriteIconOptions,
        .yOffset = 9,
    },
    [VOL_START_MENU_FLAG] =
    {
        .menuName = COMPOUND_STRING("Retire"),
        .unlockedFunc = Vol_UnlockedFunc_SafariFlag,
        .selectedFunc = Vol_SelectedFunc_SafariFlag,
        .iconTemplate = &gSpriteIconFlag,
        .yOffset = 5,
    },
};

static const u8 *const gDayNameStringsTableShortned[WEEKDAY_COUNT] =
{
    [WEEKDAY_SUN] = COMPOUND_STRING("Sun,"),
    [WEEKDAY_MON] = COMPOUND_STRING("Mon,"),
    [WEEKDAY_TUE] = COMPOUND_STRING("Tue,"),
    [WEEKDAY_WED] = COMPOUND_STRING("Wed,"),
    [WEEKDAY_THU] = COMPOUND_STRING("Thu,"),
    [WEEKDAY_FRI] = COMPOUND_STRING("Fri,"),
    [WEEKDAY_SAT] = COMPOUND_STRING("Sat,"),
};

static const u8 *const gDayNameStringsTable[WEEKDAY_COUNT] =
{
    [WEEKDAY_SUN] = COMPOUND_STRING("Sunday,"),
    [WEEKDAY_MON] = COMPOUND_STRING("Monday,"),
    [WEEKDAY_TUE] = COMPOUND_STRING("Tuesday,"),
    [WEEKDAY_WED] = COMPOUND_STRING("Wednesday,"),
    [WEEKDAY_THU] = COMPOUND_STRING("Thursday,"),
    [WEEKDAY_FRI] = COMPOUND_STRING("Friday,"),
    [WEEKDAY_SAT] = COMPOUND_STRING("Saturday,"),
};

static void Vol_SetFirstSelectedMenu(void)
{
    for (enum VolMenuItems menuOption = VOL_START_MENU_FIRST_OPTION; menuOption < VOL_START_MENU_COUNT; menuOption++)
    {
        if (sVolOptions[menuOption].unlockedFunc && sVolOptions[menuOption].unlockedFunc())
        {
            menuSelected = menuOption;
            return;
        }
    }
}

// Wally's Journey: the Pokédex icon asks "Pokédex or DexNav?" (no new icon art needed)
static const struct WindowTemplate sWindowTemplate_DexChoice = {
    .bg = 0,
    .tilemapLeft = 18,
    .tilemapTop = 1,
    .width = 7,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x80
};

#define DEX_CHOICE_FRAME_TILE STD_WINDOW_BASE_TILE_NUM
#define DEX_CHOICE_FRAME_PAL  13

static const struct MenuAction sDexChoiceActions[] = {
    {COMPOUND_STRING("Pokédex"), {NULL}},
    {COMPOUND_STRING("DexNav"),  {NULL}},
};

static void ShowSafariBallsWindow(void)
{
    sVol_StartMenu->windowIdSafariBalls = AddWindow(&sWindowTemplate_SafariBalls);
    FillWindowPixelBuffer(sVol_StartMenu->windowIdSafariBalls, PIXEL_FILL(TEXT_COLOR_WHITE));
    PutWindowTilemap(sVol_StartMenu->windowIdSafariBalls);
    ConvertIntToDecimalStringN(gStringVar1, gNumSafariBalls, STR_CONV_MODE_RIGHT_ALIGN, 2);
    StringExpandPlaceholders(gStringVar4, gText_SafariBallStock);
    AddTextPrinterParameterized(sVol_StartMenu->windowIdSafariBalls, FONT_NARROW, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sVol_StartMenu->windowIdSafariBalls, COPYWIN_GFX);
}

void Vol_StartMenu_Init(void)
{
    if (!IsOverworldLinkActive())
    {
        FreezeObjectEvents();
        PlayerFreeze();
        StopPlayerAvatar();
    }

    HideMapNamePopUpWindow();
    LockPlayerFieldControls();

    if (sVol_StartMenu == NULL)
    {
        sVol_StartMenu = AllocZeroed(sizeof(struct Vol_StartMenu));
    }

    if (sVol_StartMenu == NULL)
    {
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        return;
    }

    sVol_StartMenu->savedCallback = CB2_ReturnToFieldWithOpenMenu;
    sVol_StartMenu->isLoading = FALSE;
    sVol_StartMenu->windowIdClock = 0;
    sVol_StartMenu->spriteFlag = FALSE;

    Vol_StartMenu_LoadSprites();
    Vol_StartMenu_CreateAllSprites();
    Vol_StartMenu_LoadBgGfx();
    Vol_StartMenu_ShowTimeWindow();
    sVol_StartMenu->windowIdMenuName = AddWindow(&sWindowTemplate_MenuName);

    if (!sVolOptions[menuSelected].unlockedFunc || !sVolOptions[menuSelected].unlockedFunc())
        Vol_SetFirstSelectedMenu();

    CreateTask(Task_Vol_StartMenu_HandleMainInput, 0);

    if (GetSafariZoneFlag())
        ShowSafariBallsWindow();

    Vol_StartMenu_UpdateMenuName();
}

static void Vol_StartMenu_LoadSprites(void)
{
    u32 index;
    LoadSpritePalette(sSpritePal_Icon);
    index = IndexOfSpritePaletteTag(TAG_ICON_PAL);
    LoadPalette(sIconPal, OBJ_PLTT_ID(index), PLTT_SIZE_4BPP); 
    LoadCompressedSpriteSheet(sSpriteSheet_Icon);
}

static void Vol_StartMenu_CreateSprite(enum VolMenuItems menuItem, enum VolSmallOptions spriteId)
{
    s32 y = ICON_COORD_Y_TOP;
    s32 yOffset = 0;
    enum VolSmallOptions optionSlotPrev = spriteId - 1;

    if (spriteId != VOL_START_MENU_OPTION_1 && sVol_StartMenu->menuSmallOptions[optionSlotPrev] != VOL_START_MENU_COUNT)
        y = ICON_HEIGHT + gSprites[sVol_StartMenu->menuSmallSpriteId[optionSlotPrev]].y - sVolOptions[sVol_StartMenu->menuSmallOptions[optionSlotPrev]].yOffset;

    yOffset += sVolOptions[menuItem].yOffset;

    sVol_StartMenu->menuSmallSpriteId[spriteId] = CreateSprite(
        sVolOptions[menuItem].iconTemplate,
        ICON_COORD_X,
        y - yOffset + ICON_GAP,
        0
    );
}

static void Vol_StartMenu_CreateAllSprites(void)
{
    enum VolSmallOptions drawn = VOL_START_MENU_OPTION_1;

    for (enum VolMenuItems menuId = VOL_START_MENU_FIRST_OPTION; menuId < VOL_START_MENU_COUNT && drawn < VOL_START_MENU_OPTION_COUNT; menuId++)
    {
        const struct VolMenuOptions *menuOption = &sVolOptions[menuId];

        if (menuOption->unlockedFunc && menuOption->unlockedFunc())
        {
            enum VolSmallOptions optionSlot = VOL_START_MENU_OPTION_1 + drawn;

            Vol_StartMenu_CreateSprite(menuId, optionSlot);
            sVol_StartMenu->menuSmallOptions[optionSlot] = menuId;
            drawn++;
        }
    }

    for (; drawn < VOL_START_MENU_OPTION_COUNT; drawn++)
    {
        sVol_StartMenu->menuSmallOptions[drawn] = VOL_START_MENU_COUNT;
        sVol_StartMenu->menuSmallSpriteId[drawn] = SPRITE_NONE;
    }
}

static void Vol_StartMenu_LoadBgGfx(void)
{
    u8* buf = GetBgTilemapBuffer(0); 
    LoadBgTilemap(0, 0, 0, 0);
    DecompressAndCopyTileDataToVram(0, sStartMenuTiles, 0, 0, 0);
    if (GetSafariZoneFlag() == FALSE)
        DecompressDataWithHeaderWram(sStartMenuTilemap, buf);

    else
        DecompressDataWithHeaderWram(sStartMenuTilemapSafari, buf);
    
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(sStartMenuPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    ScheduleBgCopyTilemapToVram(0);
}

static void Vol_StartMenu_ShowTimeWindow(void)
{
    sVol_StartMenu->windowIdClock = AddWindow(&sWindowTemplate_StartClock);
    FillWindowPixelBuffer(sVol_StartMenu->windowIdClock, PIXEL_FILL(TEXT_COLOR_WHITE));
    PutWindowTilemap(sVol_StartMenu->windowIdClock);

    Vol_StartMenu_PrintClockDisplay();
}

static void Vol_StartMenu_PrintClockDisplay(void)
{
    const u8 *const *weekdayNames = gDayNameStringsTable;
    u8 weekdayFont = FONT_NARROW;
    u8 yOffset = 1;
    u8 xOffset = 0;

    if (VOL_START_SHORTENED_NAME)
    {
        weekdayNames = gDayNameStringsTableShortned;
        weekdayFont = FONT_NORMAL;
        yOffset = 0;
        xOffset = 3;
    }

    if (VOL_START_24_HOUR_MODE)
        xOffset = 3;

    // Wally's Journey: gLocalTime.days is negative when the clock is earlier than the save's
    // start date. "days % 7" then indexed outside the table and crashed, and GetDayOfWeek()
    // loops for ages on negative days, so work the weekday out here. Day 0 is a Saturday
    // (gGen3Epoch), the same as the rest of the game.
    RtcCalcLocalTime();
    s32 dayMod = gLocalTime.days % WEEKDAY_COUNT;
    if (dayMod < 0)
        dayMod += WEEKDAY_COUNT;
    u32 weekday = (WEEKDAY_SAT + dayMod) % WEEKDAY_COUNT;
    FillWindowPixelBuffer(sVol_StartMenu->windowIdClock, PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized(sVol_StartMenu->windowIdClock, weekdayFont,
        weekdayNames[weekday], 1 + xOffset, 1 - yOffset, TEXT_SKIP_DRAW, NULL
    );

    u8 time[24];
    FormatDecimalTimeWithoutSeconds(time, gLocalTime.hours, gLocalTime.minutes, VOL_START_24_HOUR_MODE);
	AddTextPrinterParameterized(sVol_StartMenu->windowIdClock, FONT_NORMAL, time,
        GetStringRightAlignXOffset(FONT_NORMAL, time, sWindowTemplate_StartClock.width * 8) - 1 - xOffset,
        1, TEXT_SKIP_DRAW, NULL
    );
    CopyWindowToVram(sVol_StartMenu->windowIdClock, COPYWIN_GFX);
}

static void Vol_StartMenu_UpdateMenuName(void)
{
    FillWindowPixelBuffer(sVol_StartMenu->windowIdMenuName, PIXEL_FILL(TEXT_COLOR_WHITE));
    PutWindowTilemap(sVol_StartMenu->windowIdMenuName);
    const u8 *optionName = sVolOptions[menuSelected].menuName;
    AddTextPrinterParameterized(sVol_StartMenu->windowIdMenuName, FONT_NORMAL, optionName,
        GetStringCenterAlignXOffset(FONT_NORMAL, optionName, sWindowTemplate_MenuName.width * 8),
        1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sVol_StartMenu->windowIdMenuName, COPYWIN_GFX);
}

static void Vol_StartMenu_ExitAndClearTilemap(void)
{
    u32 i;
    u8 *buf = GetBgTilemapBuffer(0);
    
    FillWindowPixelBuffer(sVol_StartMenu->windowIdMenuName, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    FillWindowPixelBuffer(sVol_StartMenu->windowIdClock, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    ClearWindowTilemap(sVol_StartMenu->windowIdMenuName);
    ClearWindowTilemap(sVol_StartMenu->windowIdClock);

    CopyWindowToVram(sVol_StartMenu->windowIdMenuName, COPYWIN_GFX);
    CopyWindowToVram(sVol_StartMenu->windowIdClock, COPYWIN_GFX);

    RemoveWindow(sVol_StartMenu->windowIdClock);
    RemoveWindow(sVol_StartMenu->windowIdMenuName);

    if (GetSafariZoneFlag() == TRUE)
    {
        FillWindowPixelBuffer(sVol_StartMenu->windowIdSafariBalls, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
        ClearWindowTilemap(sVol_StartMenu->windowIdSafariBalls); 
        CopyWindowToVram(sVol_StartMenu->windowIdSafariBalls, COPYWIN_GFX);
        RemoveWindow(sVol_StartMenu->windowIdSafariBalls);
    }

    for (i=0; i<2048; i++)
    {
        buf[i] = 0;
    }
    ScheduleBgCopyTilemapToVram(0);

    // Wally's Journey: the menu replaced BG palettes 14 (window frame) and 15 (message box) with its
    // own. Put the field's message box and window frame graphics back, so a box shown right after the
    // menu closes (Save, Safari Retire, a sign or NPC) has its normal frame and colours.
    LoadMessageBoxAndBorderGfx();

    for (enum VolSmallOptions spriteId = VOL_START_MENU_OPTION_1; spriteId < VOL_START_MENU_OPTION_COUNT; spriteId++)
    {
        if (sVol_StartMenu->menuSmallSpriteId[spriteId] != SPRITE_NONE)
        {
            FreeSpriteOamMatrix(&gSprites[sVol_StartMenu->menuSmallSpriteId[spriteId]]);
            DestroySprite(&gSprites[sVol_StartMenu->menuSmallSpriteId[spriteId]]);
        }
    }

    if (sVol_StartMenu != NULL)
    {
        FreeSpriteTilesByTag(TAG_ICON_GFX);  
        Free(sVol_StartMenu);
        sVol_StartMenu = NULL;
    }

    ScriptUnfreezeObjectEvents();  
    UnlockPlayerFieldControls();
}

static void DoCleanUpAndChangeCallback(MainCallback callback)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput));
        PlayRainStoppingSoundEffect();
        Vol_StartMenu_ExitAndClearTilemap();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(callback);
        gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;
    }
}


static bool32 Vol_UnlockedFunc_Unlocked(void)
{
    return TRUE;
}

static bool32 Vol_UnlockedFunc_Pokedex(void)
{
    return FlagGet(FLAG_SYS_POKEDEX_GET);
}

static bool32 Vol_UnlockedFunc_Pokemon(void)
{
    return FlagGet(FLAG_SYS_POKEMON_GET);
}

static bool32 Vol_UnlockedFunc_PokeNav(void)
{
    return FlagGet(FLAG_SYS_POKENAV_GET) && !GetSafariZoneFlag();
}

static bool32 Vol_UnlockedFunc_Save(void)
{
    return !GetSafariZoneFlag();
}

static bool32 Vol_UnlockedFunc_SafariFlag(void)
{
    return GetSafariZoneFlag();
}

static void Vol_SelectedFunc_Pokedex(void)
{
    DoCleanUpAndChangeCallback(CB2_OpenPokedex);
}

bool32 Vol_StartMenu_ShouldUse(void)
{
    // places where the old menu has entries Vol's menu doesn't (Retire, Pyramid Bag, link options)
    return !IsOverworldLinkActive()
        && !InUnionRoom()
        && !InBattlePike()
        && CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE
        && !InMultiPartnerRoom();
}

static bool32 Vol_DexNavAvailable(void)
{
    return DN_FLAG_DEXNAV_GET != 0 && FlagGet(DN_FLAG_DEXNAV_GET);
}

static void Vol_CloseDexChoice(void)
{
    ClearStdWindowAndFrameToTransparent(sVol_StartMenu->windowIdDexChoice, TRUE);
    RemoveWindow(sVol_StartMenu->windowIdDexChoice);
}

static void Task_Vol_WaitFadeOpenDexNav(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    PlayRainStoppingSoundEffect();
    Vol_StartMenu_ExitAndClearTilemap();
    DestroyTask(taskId);
    CreateTask(Task_OpenDexNavFromStartMenu, 0);
}

static void Task_Vol_StartMenu_DexChoice(u8 taskId)
{
    s8 choice = Menu_ProcessInputNoWrap();

    if (choice == MENU_NOTHING_CHOSEN)
        return;

    if (choice == MENU_B_PRESSED)
    {
        PlaySE(SE_SELECT);
        Vol_CloseDexChoice();
        gTasks[taskId].func = Task_Vol_StartMenu_HandleMainInput;
        return;
    }

    if (choice == 1 && MapHasNoEncounterData())
    {
        PlaySE(SE_FAILURE); // nothing to search for here, same rule as the old menu
        return;
    }

    Vol_CloseDexChoice();
    if (choice == 0)
    {
        // Pokédex: same as before
        FadeScreen(FADE_TO_BLACK, 0);
        sVol_StartMenu->isLoading = TRUE;
        gTasks[taskId].func = Task_Vol_StartMenu_HandleMainInput;
    }
    else
    {
        // DexNav: fade, close this menu, then the same launch the old start menu used
        FadeScreen(FADE_TO_BLACK, 0);
        gTasks[taskId].func = Task_Vol_WaitFadeOpenDexNav;
    }
}

static void Vol_OpenDexChoice(u8 taskId)
{
    u32 windowId = AddWindow(&sWindowTemplate_DexChoice);
    sVol_StartMenu->windowIdDexChoice = windowId;
    LoadUserWindowBorderGfx(windowId, DEX_CHOICE_FRAME_TILE, BG_PLTT_ID(DEX_CHOICE_FRAME_PAL));
    DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, DEX_CHOICE_FRAME_TILE, DEX_CHOICE_FRAME_PAL);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_WHITE));
    PrintMenuTable(windowId, ARRAY_COUNT(sDexChoiceActions), sDexChoiceActions);
    InitMenuInUpperLeftCornerNormal(windowId, ARRAY_COUNT(sDexChoiceActions), 0);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    gTasks[taskId].func = Task_Vol_StartMenu_DexChoice;
}

static void Vol_SelectedFunc_Pokemon(void)
{
    DoCleanUpAndChangeCallback(CB2_PartyMenuFromStartMenu);
}

static void Vol_SelectedFunc_Bag(void)
{
    DoCleanUpAndChangeCallback(CB2_BagMenuFromStartMenu);
}

static void Vol_SelectedFunc_PokeNav(void)
{
    DoCleanUpAndChangeCallback(CB2_InitPokeNav);
}

static void Vol_SelectedFunc_Trainer(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        Vol_StartMenu_ExitAndClearTilemap();
        CleanupOverworldWindowsAndTilemaps();
        if (IsOverworldLinkActive() || InUnionRoom())
        {
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card
            DestroyTask(FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput));
        }
        else if (FlagGet(FLAG_SYS_FRONTIER_PASS))
        {
            ShowFrontierPass(CB2_ReturnToFieldWithOpenMenu); // Display frontier pass
            DestroyTask(FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput));
        }
        else
        {
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card
            DestroyTask(FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput));
        }
    }
}

static void Vol_SelectedFunc_Save(void)
{
    if (!gPaletteFade.active)
    {
        Vol_StartMenu_ExitAndClearTilemap();
        FreezeObjectEvents();
        // Wally's Journey fix (owner report 2026-11-04, "save menu kinda glitched"): this used to call
        // LoadUserWindowBorderGfx(sVol_StartMenu->windowIdSaveInfo, ...) here, but
        // Vol_StartMenu_ExitAndClearTilemap had already freed sVol_StartMenu, so the frame was loaded
        // for a garbage window and the save info / Yes-No boxes had broken borders. The exit function
        // now reloads the message box and frame graphics itself.
        LockPlayerFieldControls();
        SaveGame();
        gTasks[FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput)].func = Task_Vol_StartMenu_WaitSaveGame;
    }
}

static void Vol_SelectedFunc_Settings(void)
{
    DoCleanUpAndChangeCallback(CB2_InitOptionMenu);
}

static void Vol_SelectedFunc_SafariFlag(void)
{
    if (!gPaletteFade.active)
    {
        Vol_StartMenu_ExitAndClearTilemap();
        FreezeObjectEvents();
        LockPlayerFieldControls();
        DestroyTask(FindTaskIdByFunc(Task_Vol_StartMenu_HandleMainInput));
        SafariZoneRetirePrompt();
    }
}

static void Vol_StartMenu_HandleInput(bool32 down)
{
    sVol_StartMenu->spriteFlag = FALSE;
    enum VolSmallOptions optionCurrent = VOL_START_MENU_OPTION_1;
    s32 offset;
    u32 nextIndex;

    offset = down ? 1 : -1;

    for (enum VolSmallOptions i = VOL_START_MENU_OPTION_1; i < VOL_START_MENU_OPTION_COUNT; i++)
    {
        if (sVol_StartMenu->menuSmallOptions[i] == menuSelected)
        {
            optionCurrent = i;
            break;
        }
    }

    nextIndex = optionCurrent + offset;
    if (nextIndex >= VOL_START_MENU_OPTION_COUNT
        || nextIndex < VOL_START_MENU_OPTION_1
        || sVol_StartMenu->menuSmallOptions[nextIndex] == VOL_START_MENU_COUNT)
    {
        PlaySE(SE_WALL_HIT);
        return;
    }

    PlaySE(SE_SELECT);
    menuSelected = sVol_StartMenu->menuSmallOptions[nextIndex];
    Vol_StartMenu_UpdateMenuName();
}

static void Task_Vol_StartMenu_HandleMainInput(u8 taskId)
{
    u32 index;
    if (sVol_StartMenu->isLoading == FALSE && !gPaletteFade.active)
    {
        index = IndexOfSpritePaletteTag(TAG_ICON_PAL);
        LoadPalette(sIconPal, OBJ_PLTT_ID(index), PLTT_SIZE_4BPP); 
    }

    if (VOL_START_UPDATE_CLOCK_DISPLAY)
        Vol_StartMenu_PrintClockDisplay();
    
    if (JOY_NEW(A_BUTTON))
    {
        if (sVol_StartMenu->isLoading == FALSE && menuSelected == VOL_START_MENU_POKEDEX && Vol_DexNavAvailable())
        {
            PlaySE(SE_SELECT);
            Vol_OpenDexChoice(taskId);
        }
        else if (sVol_StartMenu->isLoading == FALSE)
        {
            if (menuSelected != VOL_START_MENU_SAVE && menuSelected != VOL_START_MENU_FLAG)
                FadeScreen(FADE_TO_BLACK, 0);
            
            sVol_StartMenu->isLoading = TRUE;
        }
    }
    else if (JOY_NEW(B_BUTTON) && sVol_StartMenu->isLoading == FALSE)
    {
        PlaySE(SE_SELECT);
        Vol_StartMenu_ExitAndClearTilemap();  
        DestroyTask(taskId);
    }
    else if (gMain.newKeys & DPAD_UP && sVol_StartMenu->isLoading == FALSE)
    {
        Vol_StartMenu_HandleInput(FALSE);
    }
    else if (gMain.newKeys & DPAD_DOWN && sVol_StartMenu->isLoading == FALSE)
    {
        Vol_StartMenu_HandleInput(TRUE);
    }
    else if (sVol_StartMenu->isLoading == TRUE)
    {
        sVolOptions[menuSelected].selectedFunc();
    }
}

static void Task_Vol_StartMenu_WaitSaveGame(u8 taskId)
{
    if (!FuncIsActiveTask(SaveGameTask))
    { 
        ClearDialogWindowAndFrameToTransparent(0, TRUE);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        SoftResetInBattlePyramid();
        DestroyTask(taskId);
    }
}
