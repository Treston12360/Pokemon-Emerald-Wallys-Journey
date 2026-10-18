Route110_TrickHousePuzzle6_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route110_TrickHousePuzzle6_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Route110_TrickHousePuzzle6_OnWarp
	.byte 0

Route110_TrickHousePuzzle6_OnTransition:
	special RotatingGate_InitPuzzle
	end

Route110_TrickHousePuzzle6_OnWarp:
	map_script_2 VAR_TEMP_0, VAR_TEMP_0, Route110_TrickHousePuzzle6_EventScript_InitPuzzle
	.2byte 0

@ NOTE: This rotating gate puzzle makes use of VAR_TEMP_0 - VAR_TEMP_5
Route110_TrickHousePuzzle6_EventScript_InitPuzzle::
	special RotatingGate_InitPuzzleAndGraphics
	end

Route110_TrickHousePuzzle6_EventScript_Scroll::
	lockall
	goto_if_eq VAR_TRICK_HOUSE_PUZZLE_6_STATE, 0, Route110_TrickHousePuzzle6_EventScript_FoundScroll
	goto Route110_TrickHousePuzzle_EventScript_ReadScrollAgain
	end

Route110_TrickHousePuzzle6_EventScript_FoundScroll::
	setvar VAR_TRICK_HOUSE_PUZZLE_6_STATE, 1
	goto Route110_TrickHousePuzzle_EventScript_FoundScroll
	end

Route110_TrickHousePuzzle6_EventScript_Sophia::
	trainerbattle_single TRAINER_SOPHIA, Route110_TrickHousePuzzle6_Text_SophiaIntro, Route110_TrickHousePuzzle6_Text_SophiaDefeat
	msgbox Route110_TrickHousePuzzle6_Text_SophiaPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle6_EventScript_Benny::
	trainerbattle_single TRAINER_BENNY, Route110_TrickHousePuzzle6_Text_BennyIntro, Route110_TrickHousePuzzle6_Text_BennyDefeat
	msgbox Route110_TrickHousePuzzle6_Text_BennyPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle6_EventScript_Sebastian::
	trainerbattle_single TRAINER_SEBASTIAN, Route110_TrickHousePuzzle6_Text_SebastianIntro, Route110_TrickHousePuzzle6_Text_SebastianDefeat
	msgbox Route110_TrickHousePuzzle6_Text_SebastianPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle6_Text_WroteSecretCodeLockOpened:
	.string "{PLAYER} wrote down the secret code\n"
	.string "on the door.\p"
	.string "“TRICK MASTER is my life.”\n"
	.string "… … … … … … … …\p"
	.string "The lock clicked open!$"

Route110_TrickHousePuzzle6_Text_SophiaIntro:
	.string "A strange house on ROUTE 110?\p"
	.string "I write a travel guide. This is\n"
	.string "going on the cover!$"

Route110_TrickHousePuzzle6_Text_SophiaDefeat:
	.string "Ooh! A tough TRAINER!\n"
	.string "What a find!$"

Route110_TrickHousePuzzle6_Text_SophiaPostBattle:
	.string "I'm having so much fun in here.\n"
	.string "I'll do it all again next time!$"

Route110_TrickHousePuzzle6_Text_BennyIntro:
	.string "Maybe my BIRD POKéMON could fly me\n"
	.string "over this wall…\p"
	.string "Don't tell the TRICK MASTER!$"

Route110_TrickHousePuzzle6_Text_BennyDefeat:
	.string "Gwaaah! Blew it!$"

Route110_TrickHousePuzzle6_Text_BennyPostBattle:
	.string "Ehehe… I lost because I tried to\n"
	.string "cheat, didn't I?$"

Route110_TrickHousePuzzle6_Text_SebastianIntro:
	.string "These spinning doors…\p"
	.string "I get seasick on boats too.\n"
	.string "Why did I come in here?$"

Route110_TrickHousePuzzle6_Text_SebastianDefeat:
	.string "Everything's spinning…\n"
	.string "I give up…$"

Route110_TrickHousePuzzle6_Text_SebastianPostBattle:
	.string "You're not dizzy at all?\n"
	.string "Or is that a poker face?$"

