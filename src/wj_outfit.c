#include "global.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "wj_outfit.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/rgb.h"

// Wally's Journey: player outfits. See include/wj_outfit.h.

#define WJ_OUTFIT_MAGIC 0xA8
#define WJ_OUTFIT_MAGIC_087 0xA7 // 0.8.7 had the 20 outfits in a different order

enum { LAYER_NONE, LAYER_SKIN, LAYER_PRIMARY, LAYER_SECONDARY };

struct WJOutfitInfo
{
    const u8 *name;
    u8 gender;
    bool8 hasBack;
    u16 owPal[16];
    u8 owLayers[16];
    u16 frontPal[16];
    u8 frontLayers[16];
    u16 backPal[16];
    u8 backLayers[16];
};

#include "data/wj_outfits.h"

struct WJColour
{
    const u8 *name;
    u8 r, g, b;
};

// The colour each layer's brightest shade turns into; darker shades scale down from it.
static const struct WJColour sColours[] =
{
    {COMPOUND_STRING("DEFAULT"), 0, 0, 0},
    {COMPOUND_STRING("RED"),     31,  6,  6},
    {COMPOUND_STRING("CRIMSON"), 24,  3,  6},
    {COMPOUND_STRING("ORANGE"),  31, 16,  4},
    {COMPOUND_STRING("GOLD"),    31, 24,  6},
    {COMPOUND_STRING("YELLOW"),  31, 30,  8},
    {COMPOUND_STRING("LIME"),    18, 31,  6},
    {COMPOUND_STRING("GREEN"),    6, 24,  8},
    {COMPOUND_STRING("FOREST"),   4, 16,  6},
    {COMPOUND_STRING("TEAL"),     4, 22, 20},
    {COMPOUND_STRING("CYAN"),     8, 28, 31},
    {COMPOUND_STRING("SKY"),     14, 22, 31},
    {COMPOUND_STRING("BLUE"),     6, 12, 31},
    {COMPOUND_STRING("NAVY"),     5,  7, 20},
    {COMPOUND_STRING("PURPLE"),  18,  8, 28},
    {COMPOUND_STRING("VIOLET"),  24, 16, 31},
    {COMPOUND_STRING("PINK"),    31, 16, 24},
    {COMPOUND_STRING("MAGENTA"), 28,  6, 22},
    {COMPOUND_STRING("BROWN"),   20, 12,  6},
    {COMPOUND_STRING("TAN"),     28, 22, 14},
    {COMPOUND_STRING("WHITE"),   31, 31, 31},
    {COMPOUND_STRING("SILVER"),  24, 24, 26},
    {COMPOUND_STRING("GRAY"),    16, 16, 18},
    {COMPOUND_STRING("BLACK"),    7,  7,  8},
};

static const struct WJColour sSkinTones[] =
{
    {COMPOUND_STRING("DEFAULT"), 0, 0, 0},
    {COMPOUND_STRING("TONE 1"), 31, 27, 23},
    {COMPOUND_STRING("TONE 2"), 31, 24, 18},
    {COMPOUND_STRING("TONE 3"), 28, 20, 14},
    {COMPOUND_STRING("TONE 4"), 23, 16, 10},
    {COMPOUND_STRING("TONE 5"), 17, 11,  7},
    {COMPOUND_STRING("TONE 6"), 12,  8,  5},
};

EWRAM_DATA u16 gWJPlayerOwPal[16] = {0};
EWRAM_DATA u16 gWJPlayerFrontPal[16] = {0};
EWRAM_DATA u16 gWJPlayerBackPal[16] = {0};
EWRAM_DATA u32 gWJPlayerFrontPicBuf[(sizeof(struct TrainerFrontPicInfo) + 3) / 4] = {0};
EWRAM_DATA u32 gWJPlayerBackPicBuf[(sizeof(struct TrainerBackPicInfo) + 3) / 4] = {0};

extern const struct TrainerFrontPicInfo gWJOutfitFrontPics[WJ_OUTFIT_COUNT];
extern const struct TrainerBackPicInfo gWJOutfitBackPics[WJ_OUTFIT_COUNT];

bool32 WJ_OutfitActive(void)
{
    return gSaveBlock3Ptr->wjOutfitMagic == WJ_OUTFIT_MAGIC && gSaveBlock3Ptr->wjOutfit < WJ_OUTFIT_COUNT;
}

u32 WJ_OutfitId(void)
{
    return gSaveBlock3Ptr->wjOutfit;
}

static void GetLayerColours(u16 *out)
{
    u32 skin = gSaveBlock3Ptr->wjOutfitSkin, c1 = gSaveBlock3Ptr->wjOutfitColour[0], c2 = gSaveBlock3Ptr->wjOutfitColour[1];
    out[LAYER_NONE] = 0xFFFF;
    out[LAYER_SKIN] = (skin && skin < ARRAY_COUNT(sSkinTones)) ? RGB(sSkinTones[skin].r, sSkinTones[skin].g, sSkinTones[skin].b) : 0xFFFF;
    out[LAYER_PRIMARY] = (c1 && c1 < ARRAY_COUNT(sColours)) ? RGB(sColours[c1].r, sColours[c1].g, sColours[c1].b) : 0xFFFF;
    out[LAYER_SECONDARY] = (c2 && c2 < ARRAY_COUNT(sColours)) ? RGB(sColours[c2].r, sColours[c2].g, sColours[c2].b) : 0xFFFF;
}

static u32 Grey(u16 c)
{
    u32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    return (r * 77 + g * 151 + b * 28) >> 8;
}

// Recolour each layer: a slot's shade is kept (its grey level relative to the layer's
// brightest slot) and multiplied by the chosen colour.
static void ApplyLayers(const u16 *base, const u8 *layers, const u16 *colours, u16 *out)
{
    u32 i, white[4] = {1, 1, 1, 1};
    for (i = 1; i < 16; i++)
    {
        u32 grey = Grey(base[i]);
        if (layers[i] && grey > white[layers[i]])
            white[layers[i]] = grey;
    }
    out[0] = base[0];
    for (i = 1; i < 16; i++)
    {
        u16 c = colours[layers[i] & 3];
        if (layers[i] == LAYER_NONE || c == 0xFFFF)
        {
            out[i] = base[i];
        }
        else
        {
            u32 grey = Grey(base[i]), w = white[layers[i]];
            u32 r = ((c & 31) * grey) / w, g = (((c >> 5) & 31) * grey) / w, b = (((c >> 10) & 31) * grey) / w;
            out[i] = RGB(min(r, 31), min(g, 31), min(b, 31));
        }
    }
}

void WJ_OutfitRefresh(void)
{
    u16 colours[4];
    const struct WJOutfitInfo *info;
    if (!WJ_OutfitActive())
        return;
    info = &sWJOutfits[gSaveBlock3Ptr->wjOutfit];
    GetLayerColours(colours);
    ApplyLayers(info->owPal, info->owLayers, colours, gWJPlayerOwPal);
    ApplyLayers(info->frontPal, info->frontLayers, colours, gWJPlayerFrontPal);
    if (info->hasBack)
        ApplyLayers(info->backPal, info->backLayers, colours, gWJPlayerBackPal);
    memcpy(gWJPlayerFrontPicBuf, &gWJOutfitFrontPics[gSaveBlock3Ptr->wjOutfit], sizeof(struct TrainerFrontPicInfo));
    memcpy(gWJPlayerBackPicBuf, &gWJOutfitBackPics[gSaveBlock3Ptr->wjOutfit], sizeof(struct TrainerBackPicInfo));
}

void WJ_OutfitOnNewGame(void)
{
    gSaveBlock3Ptr->wjOutfitMagic = 0;
    gSaveBlock3Ptr->wjOutfit = 0;
    gSaveBlock3Ptr->wjOutfitSkin = 0;
    gSaveBlock3Ptr->wjOutfitColour[0] = 0;
    gSaveBlock3Ptr->wjOutfitColour[1] = 0;
    if (!CheckBagHasItem(ITEM_OUTFIT_CASE, 1))
        AddBagItem(ITEM_OUTFIT_CASE, 1);
}

void WJ_OutfitOnLoad(void)
{
    if (gSaveBlock3Ptr->wjOutfitMagic == WJ_OUTFIT_MAGIC_087)
    {
        // 0.8.7 had the same 20 outfits in a different order.
        static const u8 sFrom087[] = {
            WJ_OUTFIT_BRENDAN, WJ_OUTFIT_MAY, WJ_OUTFIT_RED, WJ_OUTFIT_LEAF, WJ_OUTFIT_ETHAN, WJ_OUTFIT_LYRA,
            WJ_OUTFIT_LUCAS, WJ_OUTFIT_DAWN, WJ_OUTFIT_HILBERT, WJ_OUTFIT_HILDA, WJ_OUTFIT_NATE, WJ_OUTFIT_ROSA,
            WJ_OUTFIT_CALEM, WJ_OUTFIT_SERENA, WJ_OUTFIT_ELIO, WJ_OUTFIT_SELENE, WJ_OUTFIT_VICTOR, WJ_OUTFIT_GLORIA,
            WJ_OUTFIT_FLORIAN, WJ_OUTFIT_JULIANA,
        };
        u32 old = gSaveBlock3Ptr->wjOutfit;
        gSaveBlock3Ptr->wjOutfitMagic = WJ_OUTFIT_MAGIC;
        gSaveBlock3Ptr->wjOutfit = (old < ARRAY_COUNT(sFrom087)) ? sFrom087[old] : ((gSaveBlock2Ptr->playerGender == MALE) ? WJ_OUTFIT_BRENDAN : WJ_OUTFIT_MAY);
    }
    if (gSaveBlock3Ptr->wjOutfitMagic != WJ_OUTFIT_MAGIC || gSaveBlock3Ptr->wjOutfit >= WJ_OUTFIT_COUNT)
        gSaveBlock3Ptr->wjOutfitMagic = 0;
    if (!CheckBagHasItem(ITEM_OUTFIT_CASE, 1))
        AddBagItem(ITEM_OUTFIT_CASE, 1);
    WJ_OutfitRefresh();
}

// The player's own sprite ids (only for the player's gender, so the rival keeps their look).
const struct ObjectEventGraphicsInfo *WJ_OutfitGraphicsInfo(u16 graphicsId)
{
    u32 slot;
    if (gSaveBlock2Ptr->playerGender == MALE)
    {
        switch (graphicsId)
        {
        case OBJ_EVENT_GFX_BRENDAN_NORMAL:     slot = WJ_OUTFIT_GFX_NORMAL; break;
        case OBJ_EVENT_GFX_BRENDAN_MACH_BIKE:  slot = WJ_OUTFIT_GFX_MACH_BIKE; break;
        case OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE:  slot = WJ_OUTFIT_GFX_ACRO_BIKE; break;
        case OBJ_EVENT_GFX_BRENDAN_SURFING:    slot = WJ_OUTFIT_GFX_SURFING; break;
        case OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE: slot = WJ_OUTFIT_GFX_FIELD_MOVE; break;
        case OBJ_EVENT_GFX_BRENDAN_FISHING:    slot = WJ_OUTFIT_GFX_FISHING; break;
        case OBJ_EVENT_GFX_BRENDAN_WATERING:   slot = WJ_OUTFIT_GFX_WATERING; break;
        case OBJ_EVENT_GFX_BRENDAN_DECORATING: slot = WJ_OUTFIT_GFX_DECORATING; break;
        default: return NULL;
        }
    }
    else
    {
        switch (graphicsId)
        {
        case OBJ_EVENT_GFX_MAY_NORMAL:     slot = WJ_OUTFIT_GFX_NORMAL; break;
        case OBJ_EVENT_GFX_MAY_MACH_BIKE:  slot = WJ_OUTFIT_GFX_MACH_BIKE; break;
        case OBJ_EVENT_GFX_MAY_ACRO_BIKE:  slot = WJ_OUTFIT_GFX_ACRO_BIKE; break;
        case OBJ_EVENT_GFX_MAY_SURFING:    slot = WJ_OUTFIT_GFX_SURFING; break;
        case OBJ_EVENT_GFX_MAY_FIELD_MOVE: slot = WJ_OUTFIT_GFX_FIELD_MOVE; break;
        case OBJ_EVENT_GFX_MAY_FISHING:    slot = WJ_OUTFIT_GFX_FISHING; break;
        case OBJ_EVENT_GFX_MAY_WATERING:   slot = WJ_OUTFIT_GFX_WATERING; break;
        case OBJ_EVENT_GFX_MAY_DECORATING: slot = WJ_OUTFIT_GFX_DECORATING; break;
        default: return NULL;
        }
    }
    return WJ_GetOutfitGfxInfo(gSaveBlock3Ptr->wjOutfit, slot);
}

// ---------------------------------------------------------------------------------------
// Outfit menu (Outfit Case key item). The player's own sprite on the map is the preview.

enum { ROW_OUTFIT, ROW_SKIN, ROW_COLOUR_1, ROW_COLOUR_2, ROW_COUNT };

#define tRow      data[0]
#define tWindowId data[1]
#define tOldMagic data[2]
#define tOldOutfit data[3]
#define tOldSkin  data[4]
#define tOldC1    data[5]
#define tOldC2    data[6]

static const struct WindowTemplate sOutfitWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 17,
    .tilemapTop = 1,
    .width = 12,
    .height = 12,
    .paletteNum = 15,
    .baseBlock = 0x100,
};

static const u8 *const sRowNames[ROW_COUNT] =
{
    COMPOUND_STRING("OUTFIT"),
    COMPOUND_STRING("SKIN"),
    COMPOUND_STRING("COLOR 1"),
    COMPOUND_STRING("COLOR 2"),
};

static bool32 OutfitHasLayer(u32 layer)
{
    u32 i;
    const struct WJOutfitInfo *info = &sWJOutfits[gSaveBlock3Ptr->wjOutfit];
    for (i = 1; i < 16; i++)
        if (info->owLayers[i] == layer)
            return TRUE;
    return FALSE;
}

static void PreviewOutfit(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 slot;
    WJ_OutfitRefresh();
    ObjectEventSetGraphicsId(player, player->graphicsId);
    slot = IndexOfSpritePaletteTag(OBJ_EVENT_PAL_TAG_WJ_PLAYER);
    if (slot != 0xFF && WJ_OutfitActive())
    {
        LoadPalette(gWJPlayerOwPal, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);
        UpdateSpritePaletteWithWeather(slot, FALSE);
    }
    ObjectEventTurn(player, DIR_SOUTH);
}

static void DrawOutfitMenu(u8 taskId)
{
    u32 row, windowId = gTasks[taskId].tWindowId;
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    for (row = 0; row < ROW_COUNT; row++)
    {
        const u8 *value;
        bool32 enabled = TRUE;
        switch (row)
        {
        case ROW_OUTFIT:
            value = sWJOutfits[gSaveBlock3Ptr->wjOutfit].name;
            break;
        case ROW_SKIN:
            value = sSkinTones[gSaveBlock3Ptr->wjOutfitSkin].name;
            enabled = OutfitHasLayer(LAYER_SKIN);
            break;
        case ROW_COLOUR_1:
            value = sColours[gSaveBlock3Ptr->wjOutfitColour[0]].name;
            enabled = OutfitHasLayer(LAYER_PRIMARY);
            break;
        default:
            value = sColours[gSaveBlock3Ptr->wjOutfitColour[1]].name;
            enabled = OutfitHasLayer(LAYER_SECONDARY);
            break;
        }
        if (row == gTasks[taskId].tRow)
            AddTextPrinterParameterized(windowId, FONT_SMALL, gText_SelectorArrow3, 0, row * 16 + 2, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_SMALL, sRowNames[row], 7, row * 16 + 2, TEXT_SKIP_DRAW, NULL);
        if (enabled)
        {
            StringCopy(gStringVar4, COMPOUND_STRING("{LEFT_ARROW}"));
            StringAppend(gStringVar4, value);
            StringAppend(gStringVar4, COMPOUND_STRING("{RIGHT_ARROW}"));
        }
        else
        {
            StringCopy(gStringVar4, COMPOUND_STRING("-"));
        }
        AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 44, row * 16 + 2, TEXT_SKIP_DRAW, NULL);
    }
    AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("{A_BUTTON} DONE {B_BUTTON} CANCEL"), 0, ROW_COUNT * 16 + 2, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_SMALL, COMPOUND_STRING("{START_BUTTON} RANDOM COLORS"), 0, ROW_COUNT * 16 + 14, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void ChangeRow(u8 taskId, s32 delta)
{
    u32 row = gTasks[taskId].tRow;
    switch (row)
    {
    case ROW_OUTFIT:
        gSaveBlock3Ptr->wjOutfit = (gSaveBlock3Ptr->wjOutfit + WJ_OUTFIT_COUNT + delta) % WJ_OUTFIT_COUNT;
        break;
    case ROW_SKIN:
        if (!OutfitHasLayer(LAYER_SKIN))
            return;
        gSaveBlock3Ptr->wjOutfitSkin = (gSaveBlock3Ptr->wjOutfitSkin + ARRAY_COUNT(sSkinTones) + delta) % ARRAY_COUNT(sSkinTones);
        break;
    case ROW_COLOUR_1:
    case ROW_COLOUR_2:
        if (!OutfitHasLayer(row == ROW_COLOUR_1 ? LAYER_PRIMARY : LAYER_SECONDARY))
            return;
        gSaveBlock3Ptr->wjOutfitColour[row - ROW_COLOUR_1] = (gSaveBlock3Ptr->wjOutfitColour[row - ROW_COLOUR_1] + ARRAY_COUNT(sColours) + delta) % ARRAY_COUNT(sColours);
        break;
    }
    PlaySE(SE_SELECT);
    PreviewOutfit();
    DrawOutfitMenu(taskId);
}

static void CloseOutfitMenu(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_OutfitMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        tRow = (tRow + ROW_COUNT - 1) % ROW_COUNT;
        DrawOutfitMenu(taskId);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        tRow = (tRow + 1) % ROW_COUNT;
        DrawOutfitMenu(taskId);
    }
    else if (JOY_REPEAT(DPAD_LEFT))
    {
        ChangeRow(taskId, -1);
    }
    else if (JOY_REPEAT(DPAD_RIGHT))
    {
        ChangeRow(taskId, 1);
    }
    else if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSaveBlock3Ptr->wjOutfitSkin = Random() % ARRAY_COUNT(sSkinTones);
        gSaveBlock3Ptr->wjOutfitColour[0] = Random() % ARRAY_COUNT(sColours);
        gSaveBlock3Ptr->wjOutfitColour[1] = Random() % ARRAY_COUNT(sColours);
        PreviewOutfit();
        DrawOutfitMenu(taskId);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseOutfitMenu(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSaveBlock3Ptr->wjOutfitMagic = tOldMagic;
        gSaveBlock3Ptr->wjOutfit = tOldOutfit;
        gSaveBlock3Ptr->wjOutfitSkin = tOldSkin;
        gSaveBlock3Ptr->wjOutfitColour[0] = tOldC1;
        gSaveBlock3Ptr->wjOutfitColour[1] = tOldC2;
        PreviewOutfit();
        CloseOutfitMenu(taskId);
    }
}

// special WJ_OutfitMenu (followed by waitstate)
void WJ_OutfitMenu(void)
{
    u8 taskId = CreateTask(Task_OutfitMenu, 80);
    s16 *data = gTasks[taskId].data;
    tOldMagic = gSaveBlock3Ptr->wjOutfitMagic;
    tOldOutfit = gSaveBlock3Ptr->wjOutfit;
    tOldSkin = gSaveBlock3Ptr->wjOutfitSkin;
    tOldC1 = gSaveBlock3Ptr->wjOutfitColour[0];
    tOldC2 = gSaveBlock3Ptr->wjOutfitColour[1];
    if (!WJ_OutfitActive())
    {
        // First time: start from the player's own Brendan/May look.
        gSaveBlock3Ptr->wjOutfitMagic = WJ_OUTFIT_MAGIC;
        gSaveBlock3Ptr->wjOutfit = (gSaveBlock2Ptr->playerGender == MALE) ? WJ_OUTFIT_BRENDAN : WJ_OUTFIT_MAY;
        gSaveBlock3Ptr->wjOutfitSkin = 0;
        gSaveBlock3Ptr->wjOutfitColour[0] = 0;
        gSaveBlock3Ptr->wjOutfitColour[1] = 0;
    }
    tRow = ROW_OUTFIT;
    tWindowId = AddWindow(&sOutfitWindowTemplate);
    DrawStdWindowFrame(tWindowId, FALSE);
    PreviewOutfit();
    DrawOutfitMenu(taskId);
}
