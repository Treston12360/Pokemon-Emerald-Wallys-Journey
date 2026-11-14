@ Project ARIA — chapter 2 (Mt. Moon). Mechanics only: the dialogue is drafted in docs/KANTO_SCRIPT.md and
@ added after the owner approves it.
@ Tears: OBJ_EVENT_GFX_ARIA_TEAR on the bottom-middle tile of a 3x3; the whole 3x3 is made solid on load
@ (owner 2026-11-14: you can't walk onto any of it), and freed again when it's sealed.

Aria_Ch2_MtMoonOnLoad::
	goto_if_set FLAG_ARIA_TEAR_MT_MOON_SEALED, Aria_Ch0_Return_End
	setmetatile 36, 20, 0x281, TRUE
	setmetatile 37, 20, 0x281, TRUE
	setmetatile 38, 20, 0x281, TRUE
	setmetatile 36, 21, 0x281, TRUE
	setmetatile 37, 21, 0x281, TRUE
	setmetatile 38, 21, 0x281, TRUE
	setmetatile 36, 22, 0x281, TRUE
	setmetatile 38, 22, 0x281, TRUE
	end

Aria_Ch0_Return_End::
	end

@ Placeholder until ch. 2 is written: with the Rift Scanner, sealing works (flash, shake, gone).
Aria_Ch2_Tear::
	lockall
	checkitem ITEM_RIFT_SCANNER
	goto_if_eq VAR_RESULT, FALSE, Aria_Ch2_TearNoScanner
	msgbox Aria_Ch2_Text_UsedScanner, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 2  @ vertical pan
	setvar VAR_0x8005, 2  @ horizontal pan
	setvar VAR_0x8006, 8  @ shakes
	setvar VAR_0x8007, 4  @ delay
	special ShakeCamera
	waitstate
	playse SE_M_REFLECT
	fadescreenspeed FADE_TO_WHITE, 4
	setflag FLAG_ARIA_TEAR_MT_MOON_SEALED
	removeobject LOCALID_MT_MOON_B2F_TEAR
	setmetatile 36, 20, 0x281, FALSE
	setmetatile 37, 20, 0x281, FALSE
	setmetatile 38, 20, 0x281, FALSE
	setmetatile 36, 21, 0x281, FALSE
	setmetatile 37, 21, 0x281, FALSE
	setmetatile 38, 21, 0x281, FALSE
	setmetatile 36, 22, 0x281, FALSE
	setmetatile 38, 22, 0x281, FALSE
	special DrawWholeMapView
	fadescreenspeed FADE_FROM_WHITE, 4
	releaseall
	end

Aria_Ch2_TearNoScanner::
	releaseall
	end

Aria_Ch2_Text_UsedScanner:: @ placeholder (system line), not script-doc text
	.string "{PLAYER} used the Rift Scanner.$"

