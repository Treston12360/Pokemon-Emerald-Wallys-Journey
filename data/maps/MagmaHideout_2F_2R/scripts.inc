MagmaHideout_2F_2R_MapScripts::
	.byte 0

MagmaHideout_2F_2R_EventScript_Grunt15::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_15, MagmaHideout_2F_2R_Text_Grunt15Intro, MagmaHideout_2F_2R_Text_Grunt15Defeat
	msgbox MagmaHideout_2F_2R_Text_Grunt15PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_2R_EventScript_Grunt6::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_6, MagmaHideout_2F_2R_Text_Grunt6Intro, MagmaHideout_2F_2R_Text_Grunt6Defeat
	msgbox MagmaHideout_2F_2R_Text_Grunt6PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_2R_EventScript_Grunt7::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_7, MagmaHideout_2F_2R_Text_Grunt7Intro, MagmaHideout_2F_2R_Text_Grunt7Defeat
	msgbox MagmaHideout_2F_2R_Text_Grunt7PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_2R_EventScript_Grunt8::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_8, MagmaHideout_2F_2R_Text_Grunt8Intro, MagmaHideout_2F_2R_Text_Grunt8Defeat
	msgbox MagmaHideout_2F_2R_Text_Grunt8PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_2R_Text_Grunt15Intro:
	.string "I'm a surveyor. I mapped every valley\n"
	.string "in HOENN for MAXIE's housing plans.\p"
	.string "The numbers work. They really do.\n"
	.string "So why do I feel sick about this?$"

MagmaHideout_2F_2R_Text_Grunt15Defeat:
	.string "I may have lost… but\n"
	.string "GROUDON won't.$"

MagmaHideout_2F_2R_Text_Grunt15PostBattle:
	.string "We have the BLUE ORB now. MAXIE\n"
	.string "says GROUDON will listen to it.\p"
	.string "…He's never been wrong before.\n"
	.string "I hope he isn't wrong now.$"

MagmaHideout_2F_2R_Text_Grunt6Intro:
	.string "This heat is awful. Maybe TEAM\n"
	.string "AQUA has air conditioning…$"

MagmaHideout_2F_2R_Text_Grunt6Defeat:
	.string "Yeah… maybe I picked\n"
	.string "the wrong TEAM.$"

MagmaHideout_2F_2R_Text_Grunt6PostBattle:
	.string "Don't you miss the sea, down in\n"
	.string "a place like this?$"

MagmaHideout_2F_2R_Text_Grunt7Intro:
	.string "Feel those tremors? Is that the\n"
	.string "volcano? Or GROU…\p"
	.string "Whoops! Never mind!$"

MagmaHideout_2F_2R_Text_Grunt7Defeat:
	.string "You battle like a volcano.\n"
	.string "I respect that.$"

MagmaHideout_2F_2R_Text_Grunt7PostBattle:
	.string "You won this time, but it doesn't\n"
	.string "matter. Our goal is close!$"

MagmaHideout_2F_2R_Text_Grunt8Intro:
	.string "One of our guys lost his MAGMA\n"
	.string "EMBLEM and freaked out…\p"
	.string "Hey! Did YOU find it?!$"

MagmaHideout_2F_2R_Text_Grunt8Defeat:
	.string "I can't believe this…$"

MagmaHideout_2F_2R_Text_Grunt8PostBattle:
	.string "I've got a bad feeling about\n"
	.string "this whole plan…$"

