MagmaHideout_3F_2R_MapScripts::
	.byte 0

MagmaHideout_3F_2R_EventScript_Grunt10::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_10, MagmaHideout_3F_2R_Text_Grunt10Intro, MagmaHideout_3F_2R_Text_Grunt10Defeat
	msgbox MagmaHideout_3F_2R_Text_Grunt10PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_3F_2R_Text_Grunt10Intro:
	.string "I follow MAXIE. I do. But…\p"
	.string "Waking an ancient POKéMON?\n"
	.string "Stealing a METEORITE?\p"
	.string "This isn't what we signed up for.\n"
	.string "Is it?$"

MagmaHideout_3F_2R_Text_Grunt10Defeat:
	.string "Yeah… we really are doing\n"
	.string "something wrong.$"

MagmaHideout_3F_2R_Text_Grunt10PostBattle:
	.string "Losing cleared my head.\p"
	.string "Next time I see MAXIE, I'm asking\n"
	.string "what we're really doing.$"

