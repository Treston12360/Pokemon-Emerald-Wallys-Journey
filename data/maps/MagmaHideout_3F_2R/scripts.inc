MagmaHideout_3F_2R_MapScripts::
	.byte 0

MagmaHideout_3F_2R_EventScript_Grunt10::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_10, MagmaHideout_3F_2R_Text_Grunt10Intro, MagmaHideout_3F_2R_Text_Grunt10Defeat
	msgbox MagmaHideout_3F_2R_Text_Grunt10PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_3F_2R_Text_Grunt10Intro:
	.string "I understand everything our leader\n"
	.string "says. But you know what?\p"
	.string "Digging up an ancient POKéMON?\n"
	.string "Stealing someone's METEORITE?\p"
	.string "I think we're going too far.\n"
	.string "What do you think?$"

MagmaHideout_3F_2R_Text_Grunt10Defeat:
	.string "Yeah, I think we're doing\n"
	.string "something wrong somehow.$"

MagmaHideout_3F_2R_Text_Grunt10PostBattle:
	.string "Losing to you cleared my mind.\p"
	.string "Next time I see MAXIE, I'm going to\n"
	.string "ask him what we're really doing.$"

