MagmaHideout_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MagmaHideout_1F_OnTransition
	.byte 0

MagmaHideout_1F_OnTransition:
	setvar VAR_JAGGED_PASS_ASH_WEATHER, 0
	end

MagmaHideout_1F_EventScript_Grunt1::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_1, MagmaHideout_1F_Text_Grunt1Intro, MagmaHideout_1F_Text_Grunt1Defeat
	msgbox MagmaHideout_1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_1F_EventScript_Grunt2::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_2, MagmaHideout_1F_Text_Grunt2Intro, MagmaHideout_1F_Text_Grunt2Defeat
	msgbox MagmaHideout_1F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_1F_Text_Grunt1Intro:
	.string "TEAM MAGMA hands out guard posts\n"
	.string "in roll-call order.\p"
	.string "I'm always late, so I'm stuck in\n"
	.string "this corner.$"

MagmaHideout_1F_Text_Grunt1Defeat:
	.string "Always late, always last.\n"
	.string "I'm not cut out for this.$"

MagmaHideout_1F_Text_Grunt1PostBattle:
	.string "My folks farm on ROUTE 113. The ash\n"
	.string "ruins half the crop every year.\p"
	.string "More land, safe land. That's all I\n"
	.string "wanted. That's why I'm here.$"

MagmaHideout_1F_Text_Grunt2Intro:
	.string "MAXIE told us to dig into MT.\n"
	.string "CHIMNEY, so we dug for months.\p"
	.string "And we found something huge!\p"
	.string "What was it? Fuhahaha!\n"
	.string "Beat me and I'll tell you!$"

MagmaHideout_1F_Text_Grunt2Defeat:
	.string "Arrgh! Dug myself\n"
	.string "into a hole!$"

MagmaHideout_1F_Text_Grunt2PostBattle:
	.string "Actually, I won't tell. You'll see\n"
	.string "it soon enough!\p"
	.string "Best to save surprises for the end.$"

