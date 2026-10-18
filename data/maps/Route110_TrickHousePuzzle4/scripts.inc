Route110_TrickHousePuzzle4_MapScripts::
	.byte 0

Route110_TrickHousePuzzle4_EventScript_Scroll::
	lockall
	goto_if_eq VAR_TRICK_HOUSE_PUZZLE_4_STATE, 0, Route110_TrickHousePuzzle4_EventScript_FoundScroll
	goto Route110_TrickHousePuzzle_EventScript_ReadScrollAgain
	end

Route110_TrickHousePuzzle4_EventScript_FoundScroll::
	setvar VAR_TRICK_HOUSE_PUZZLE_4_STATE, 1
	goto Route110_TrickHousePuzzle_EventScript_FoundScroll
	end

Route110_TrickHousePuzzle4_EventScript_Cora::
	trainerbattle_single TRAINER_CORA, Route110_TrickHousePuzzle4_Text_CoraIntro, Route110_TrickHousePuzzle4_Text_CoraDefeat
	msgbox Route110_TrickHousePuzzle4_Text_CoraPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle4_EventScript_Yuji::
	trainerbattle_single TRAINER_YUJI, Route110_TrickHousePuzzle4_Text_YujiIntro, Route110_TrickHousePuzzle4_Text_YujiDefeat
	msgbox Route110_TrickHousePuzzle4_Text_YujiPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle4_EventScript_Paula::
	trainerbattle_single TRAINER_PAULA, Route110_TrickHousePuzzle4_Text_PaulaIntro, Route110_TrickHousePuzzle4_Text_PaulaDefeat
	msgbox Route110_TrickHousePuzzle4_Text_PaulaPostBattle, MSGBOX_AUTOCLOSE
	end

Route110_TrickHousePuzzle4_Text_WroteSecretCodeLockOpened:
	.string "{PLAYER} wrote down the secret code\n"
	.string "on the door.\p"
	.string "“TRICK MASTER is cool.”\n"
	.string "… … … … … … … …\p"
	.string "The lock clicked open!$"

Route110_TrickHousePuzzle4_Text_CoraIntro:
	.string "Puzzles? Ugh!\p"
	.string "I just came in because I heard\n"
	.string "TRAINERS were here. Battle!$"

Route110_TrickHousePuzzle4_Text_CoraDefeat:
	.string "I lost, but battling is\n"
	.string "still the best part!$"

Route110_TrickHousePuzzle4_Text_CoraPostBattle:
	.string "You'd go anywhere for a battle\n"
	.string "too, right? Right?$"

Route110_TrickHousePuzzle4_Text_YujiIntro:
	.string "I move boulders at the RUSTBORO\n"
	.string "quarry for a living!\p"
	.string "One finger! Watch!$"

Route110_TrickHousePuzzle4_Text_YujiDefeat:
	.string "I can move boulders, but\n"
	.string "not this puzzle…$"

Route110_TrickHousePuzzle4_Text_YujiPostBattle:
	.string "Being brawny isn't enough…\n"
	.string "You have to use your head too!$"

Route110_TrickHousePuzzle4_Text_PaulaIntro:
	.string "The TRICK HOUSE gets trickier every\n"
	.string "time.\p"
	.string "I think the TRICK MASTER reads our\n"
	.string "minds. Battle!$"

Route110_TrickHousePuzzle4_Text_PaulaDefeat:
	.string "Aaak! Tricked by\n"
	.string "another TRAINER!$"

Route110_TrickHousePuzzle4_Text_PaulaPostBattle:
	.string "Has anyone actually made it to\n"
	.string "the end? Ever?$"

