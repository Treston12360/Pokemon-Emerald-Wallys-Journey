SeafloorCavern_Room4_MapScripts::
	.byte 0

SeafloorCavern_Room4_EventScript_Grunt3::
	trainerbattle_single TRAINER_GRUNT_SEAFLOOR_CAVERN_3, SeafloorCavern_Room4_Text_Grunt3Intro, SeafloorCavern_Room4_Text_Grunt3Defeat
	msgbox SeafloorCavern_Room4_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room4_EventScript_Grunt4::
	trainerbattle_single TRAINER_GRUNT_SEAFLOOR_CAVERN_4, SeafloorCavern_Room4_Text_Grunt4Intro, SeafloorCavern_Room4_Text_Grunt4Defeat
	msgbox SeafloorCavern_Room4_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room4_Text_Grunt3Intro:
	.string "Who are you?! How did you get\n"
	.string "down here?$"

SeafloorCavern_Room4_Text_Grunt3Defeat:
	.string "Lost it… and I still\n"
	.string "can't find the exit.$"

SeafloorCavern_Room4_Text_Grunt3PostBattle:
	.string "I can't find the way out!\p"
	.string "I'm not scared. Don't get me wrong!$"

SeafloorCavern_Room4_Text_Grunt4Intro:
	.string "Who are you? Where do you think\n"
	.string "you're going?$"

SeafloorCavern_Room4_Text_Grunt4Defeat:
	.string "I couldn't stop you!$"

SeafloorCavern_Room4_Text_Grunt4PostBattle:
	.string "My partner left the map in the\n"
	.string "submarine!\p"
	.string "How useless is that?$"

