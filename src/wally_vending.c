// Wally's Journey: choose how many drinks to buy from the Lilycove Department
// Store rooftop vending machine (owner request 2026-10-29).
//
// Script use (see LilycoveCity_DepartmentStoreRooftop/scripts.inc):
//   VAR_0x8004 = item, VAR_0x8005 = price of one
//   special WallyJourney_VendingChooseQuantity / waitstate -> VAR_RESULT = amount (0 = cancelled)
//   special WallyJourney_VendingBuy -> pays for and adds VAR_RESULT of the item,
//                                      STR_VAR_1 = the amount as text
// Controls match the Mart: Up/Down +-1, Right/Left +-10, A buys, B cancels.
#include "global.h"
#include "event_data.h"
#include "international_string_util.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

#define VENDING_MAX_PER_PURCHASE 99

#define tQuantity data[0]
#define tMax      data[1]
#define tWindowId data[2]

static u32 VendingMaxQuantity(u32 item, u32 price)
{
    u32 max = VENDING_MAX_PER_PURCHASE;
    u32 space = GetFreeSpaceForItemInBag(item);
    if (price != 0 && GetMoney(&gSaveBlock1Ptr->money) / price < max)
        max = GetMoney(&gSaveBlock1Ptr->money) / price;
    if (space < max)
        max = space;
    return max;
}

static void PrintVendingQuantity(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 windowId = tWindowId;
    u32 total = (u32)tQuantity * gSpecialVar_0x8005;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    ConvertIntToDecimalStringN(gStringVar1, tQuantity, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringExpandPlaceholders(gStringVar4, gText_xVar1);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(gStringVar1, total, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringExpandPlaceholders(gStringVar4, gText_PokedollarVar1);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4,
                                GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 88), 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void CloseVending(u8 taskId, u16 result)
{
    s16 *data = gTasks[taskId].data;
    ClearStdWindowAndFrameToTransparent(tWindowId, FALSE);
    CopyWindowToVram(tWindowId, COPYWIN_GFX);
    RemoveWindow(tWindowId);
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_VendingChooseQuantity(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tQuantity, tMax) == TRUE)
    {
        PrintVendingQuantity(taskId);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseVending(taskId, tQuantity);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseVending(taskId, 0);
    }
}

void WallyJourney_VendingChooseQuantity(void)
{
    struct WindowTemplate template;
    u8 taskId;
    s16 *data;
    u32 max = VendingMaxQuantity(gSpecialVar_0x8004, gSpecialVar_0x8005);

    if (max == 0)
    {
        gSpecialVar_Result = 0; // the script already checked money and space for one
        ScriptContext_Enable();
        return;
    }

    // Below the money box (top left); its tiles end at 8 + 20, multichoice uses 100+.
    SetWindowTemplateFields(&template, 0, 2, 5, 11, 2, 15, 0x30);
    taskId = CreateTask(Task_VendingChooseQuantity, 80);
    data = gTasks[taskId].data;
    tQuantity = 1;
    tMax = max;
    tWindowId = AddWindow(&template);
    DrawStdFrameWithCustomTileAndPalette(tWindowId, FALSE, 0x214, 14);
    PutWindowTilemap(tWindowId);
    PrintVendingQuantity(taskId);
}

void WallyJourney_VendingBuy(void)
{
    u32 item = gSpecialVar_0x8004;
    u32 price = gSpecialVar_0x8005;
    u32 count = gSpecialVar_Result;

    if (count > VendingMaxQuantity(item, price))
        count = VendingMaxQuantity(item, price);
    RemoveMoney(&gSaveBlock1Ptr->money, count * price);
    AddBagItem(item, count);
    gSpecialVar_Result = count;
    ConvertIntToDecimalStringN(gStringVar1, count, STR_CONV_MODE_LEFT_ALIGN, 2);
}

#undef tQuantity
#undef tMax
#undef tWindowId
