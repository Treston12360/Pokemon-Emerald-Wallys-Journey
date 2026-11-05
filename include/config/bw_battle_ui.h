#ifndef GUARD_CONFIG_BW_BATTLE_UI_H
#define GUARD_CONFIG_BW_BATTLE_UI_H

// General identifier that may be referenced in other feature branches.
// Wally's Journey: the BW UI can be switched back to Emerald's in Options → Battle Screen,
// so these are runtime checks now (WJ_BwBattleUi in wally_journey.c).
int WJ_BwBattleUi(void);
#define BW_BATTLE_UI                (WJ_BwBattleUi())

// Specific toggle for mix-n-match ui elements.
// This is NOT recommended if you want to use this as a base for your own UI!

#define BW_BATTLE_UI_TEXTBOX        (WJ_BwBattleUi())  // Enables BW1-styled textbox. If BW_BATTLE_UI_INPUTBOX is FALSE, uses gen3-styled navigation menu instead.
#define BW_BATTLE_UI_INPUTBOX       (WJ_BwBattleUi())  // Various elements related to navigation e.g. executing a move, use an item, or running away. Requires BW_BATTLE_UI_TEXTBOX.
#define BW_BATTLE_UI_PARTY_SUMMARY  (WJ_BwBattleUi())  // Party summary bar, typically ball_status_bar and ball_display.
#define BW_BATTLE_UI_HEALTHBOX      (WJ_BwBattleUi())  // The real jam. Includes custom status ailment gfx and outlined font as well.
#define BW_BATTLE_UI_ABILITY_POP_UP (WJ_BwBattleUi())  // Another real jam. Replaces expansion's functionality, includes customized animation.
#define BW_BATTLE_UI_WINDOW_SPRITES (WJ_BwBattleUi())  // Both Last Used Ball and Move Description trigger sprites are handled here.

// Config safeguards, do not delete unless you know what you're doing!
#if (B_MOVE_REARRANGEMENT_IN_BATTLE < GEN_4)
#error "pokeemerald-expansion's `B_MOVE_REARRANGEMENT_IN_BATTLE' config is unsupported by `BW_BATTLE_UI'!"
#endif


#endif // GUARD_CONFIG_BW_BATTLE_UI_H
