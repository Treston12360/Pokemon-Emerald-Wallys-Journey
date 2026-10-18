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
	.string "I may have lost, but…$"

MagmaHideout_2F_2R_Text_Grunt15PostBattle:
	.string "We have the BLUE ORB now. MAXIE\n"
	.string "says GROUDON will listen to it.\p"
	.string "…He's never been wrong before.\n"
	.string "I hope he isn't wrong now.$"

MagmaHideout_2F_2R_Text_Grunt6Intro:
	.string "I can't stand heat. Maybe I\n"
	.string "should've joined TEAM AQUA…$"

MagmaHideout_2F_2R_Text_Grunt6Defeat:
	.string "Yeah, maybe I'm not right for\n"
	.string "TEAM MAGMA…$"

MagmaHideout_2F_2R_Text_Grunt6PostBattle:
	.string "Don't you miss the sea, stuck in\n"
	.string "a place like this?$"

MagmaHideout_2F_2R_Text_Grunt7Intro:
	.string "You hear tremors here sometimes.\p"
	.string "Is it the volcano?\n"
	.string "Or is it GROU…\p"
	.string "Whoops!\n"
	.string "No, no, never mind!$"

MagmaHideout_2F_2R_Text_Grunt7Defeat:
	.string "You're a fiery battler.\n"
	.string "Just like a volcano!$"

MagmaHideout_2F_2R_Text_Grunt7PostBattle:
	.string "You won this time, but it doesn't\n"
	.string "matter. Our goal is close!$"

MagmaHideout_2F_2R_Text_Grunt8Intro:
	.string "One of our guys was freaking out\n"
	.string "about losing his MAGMA EMBLEM…\p"
	.string "Wait a minute!\n"
	.string "Was it you who found it?$"

MagmaHideout_2F_2R_Text_Grunt8Defeat:
	.string "I'm having trouble believing this…$"

MagmaHideout_2F_2R_Text_Grunt8PostBattle:
	.string "I'm getting a feeling our plan\n"
	.string "is going to end in failure…$"

