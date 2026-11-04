#ifndef GUARD_UI_STAT_EDIT_H
#define GUARD_UI_STAT_EDIT_H

#include "main.h"

void Task_OpenStatEditorFromStartMenu(u8 taskId);
void StatEditor_Init(MainCallback callback);
// Wally's Journey: every level is worth 10 stat points, up to MAX_TOTAL_EVS (510, reached at Lv51)
#define STAT_EDITOR_POINTS_PER_LEVEL 10
bool32 StatEditor_LevelUpGivesPoints(u32 oldLevel, u32 newLevel);

extern const u8 *const gNatureNamePointers[];
extern const u8 gAbilityNames[][ABILITY_NAME_LENGTH + 1];
extern const struct SpeciesInfo gSpeciesInfo[];


#endif // GUARD_UI_MENU_H
