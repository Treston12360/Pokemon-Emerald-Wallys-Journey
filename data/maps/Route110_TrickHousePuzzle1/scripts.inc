Route110_TrickHousePuzzle1_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Route110_TrickHousePuzzle1_OnLoad
	.byte 0

Route110_TrickHousePuzzle1_OnLoad:
	goto_if_eq VAR_TRICK_HOUSE_PUZZLE_1_STATE, 2, Route110_TrickHousePuzzle1_EventScript_OpenDoor
	end

Route110_TrickHousePuzzle1_EventScript_OpenDoor::
	setmetatile 13, 1, METATILE_TrickHousePuzzle_Stairs_Down, FALSE
	end

Route110_TrickHousePuzzle1_EventScript_Scroll::
	lockall
	goto_if_eq VAR_TRICK_HOUSE_PUZZLE_1_STATE, 0, Route110_TrickHousePuzzle1_EventScript_FoundScroll
	goto Route110_TrickHousePuzzle_EventScript_ReadScrollAgain
	end

Route110_TrickHousePuzzle1_EventScript_FoundScroll::
	setvar VAR_TRICK_HOUSE_PUZZLE_1_STATE, 1
	goto Route110_TrickHousePuzzle_EventScript_FoundScroll
	end

Route110_TrickHousePuzzle1_EventScript_Sally::
	trainerbattle_single TRAINER_SALLY, Route110_TrickHousePuzzle1_Text_SallyIntro, Route110_TrickHousePuzzle1_Text_SallyDefeat
	msgbox Route110_TrickHousePuzzle1_Text_SallyPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle1_EventScript_Eddie::
	trainerbattle_single TRAINER_EDDIE, Route110_TrickHousePuzzle1_Text_EddieIntro, Route110_TrickHousePuzzle1_Text_EddieDefeat
	msgbox Route110_TrickHousePuzzle1_Text_EddiePostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle1_EventScript_Robin::
	trainerbattle_single TRAINER_ROBIN, Route110_TrickHousePuzzle1_Text_RobinIntro, Route110_TrickHousePuzzle1_Text_RobinDefeat
	msgbox Route110_TrickHousePuzzle1_Text_RobinPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle1_Text_WroteSecretCodeLockOpened::
	.string "{PLAYER} wrote down the secret code\n"
	.string "on the door.\p"
	.string "“TRICK MASTER is fabulous.”\n"
	.string "… … … … … … … …\p"
	.string "The lock clicked open!$"

Route110_TrickHousePuzzle1_Text_SallyIntro:
	.string "I just got my STONE BADGE, so I can\n"
	.string "CUT trees now! Watch out!$"

Route110_TrickHousePuzzle1_Text_SallyDefeat:
	.string "Hey! You're taking this\n"
	.string "way too seriously!$"

Route110_TrickHousePuzzle1_Text_SallyPostBattle:
	.string "I've cut every tree in here twice.\n"
	.string "They grow back overnight! How?!$"

Route110_TrickHousePuzzle1_Text_EddieIntro:
	.string "I came in to ask for directions to\n"
	.string "SLATEPORT. Now I'm trapped!$"

Route110_TrickHousePuzzle1_Text_EddieDefeat:
	.string "Lost my way AND the match…$"

Route110_TrickHousePuzzle1_Text_EddiePostBattle:
	.string "If you get out, tell my mom I'm\n"
	.string "fine. Mostly fine.$"

Route110_TrickHousePuzzle1_Text_RobinIntro:
	.string "Who IS the TRICK MASTER? I've been\n"
	.string "investigating for weeks!$"

Route110_TrickHousePuzzle1_Text_RobinDefeat:
	.string "I was thinking about the\n"
	.string "mystery, not the battle!$"

Route110_TrickHousePuzzle1_Text_RobinPostBattle:
	.string "You're strong! Wait… are YOU the\n"
	.string "TRICK MASTER in disguise?$"

