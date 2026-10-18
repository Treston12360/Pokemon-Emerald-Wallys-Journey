Route110_TrickHousePuzzle8_MapScripts::
	.byte 0

Route110_TrickHousePuzzle8_EventScript_Scroll::
	lockall
	goto_if_eq VAR_TRICK_HOUSE_PUZZLE_8_STATE, 0, Route110_TrickHousePuzzle8_EventScript_FoundScroll
	goto Route110_TrickHousePuzzle_EventScript_ReadScrollAgain
	end

Route110_TrickHousePuzzle8_EventScript_FoundScroll::
	setvar VAR_TRICK_HOUSE_PUZZLE_8_STATE, 1
	goto Route110_TrickHousePuzzle_EventScript_FoundScroll
	end

Route110_TrickHousePuzzle8_EventScript_Vincent::
	trainerbattle_single TRAINER_VINCENT, Route110_TrickHousePuzzle8_Text_VincentIntro, Route110_TrickHousePuzzle8_Text_VincentDefeat
	msgbox Route110_TrickHousePuzzle8_Text_VincentPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle8_EventScript_Keira::
	trainerbattle_single TRAINER_KEIRA, Route110_TrickHousePuzzle8_Text_KeiraIntro, Route110_TrickHousePuzzle8_Text_KeiraDefeat
	msgbox Route110_TrickHousePuzzle8_Text_KeiraPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle8_EventScript_Leroy::
	trainerbattle_single TRAINER_LEROY, Route110_TrickHousePuzzle8_Text_LeroyIntro, Route110_TrickHousePuzzle8_Text_LeroyDefeat
	msgbox Route110_TrickHousePuzzle8_Text_LeroyPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle8_Text_WroteSecretCodeLockOpened::
	.string "{PLAYER} wrote down the secret code\n"
	.string "on the door.\p"
	.string "“TRICK MASTER I love.”\n"
	.string "… … … … … … … …\p"
	.string "The lock clicked open!$"

Route110_TrickHousePuzzle8_Text_VincentIntro:
	.string "Not many people make it this far.\n"
	.string "Respect.$"

Route110_TrickHousePuzzle8_Text_VincentDefeat:
	.string "So you're tough too…$"

Route110_TrickHousePuzzle8_Text_VincentPostBattle:
	.string "You beat the POKéMON LEAGUE\n"
	.string "CHAMPION? No wonder!$"

Route110_TrickHousePuzzle8_Text_KeiraIntro:
	.string "You should feel lucky to battle\n"
	.string "someone like me!$"

Route110_TrickHousePuzzle8_Text_KeiraDefeat:
	.string "This is wrong!\n"
	.string "I don't lose!$"

Route110_TrickHousePuzzle8_Text_KeiraPostBattle:
	.string "Beating me was a miracle.\n"
	.string "Go ahead, brag.$"

Route110_TrickHousePuzzle8_Text_LeroyIntro:
	.string "Another TRICK HOUSE veteran?\n"
	.string "You've suffered too, then.$"

Route110_TrickHousePuzzle8_Text_LeroyDefeat:
	.string "I see. A remarkable style.$"

Route110_TrickHousePuzzle8_Text_LeroyPostBattle:
	.string "The TRICK MASTER must be thrilled\n"
	.string "to have a challenger like you.$"

