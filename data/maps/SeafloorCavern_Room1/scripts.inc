SeafloorCavern_Room1_MapScripts::
	.byte 0

SeafloorCavern_Room1_EventScript_Grunt1::
	trainerbattle_single TRAINER_GRUNT_SEAFLOOR_CAVERN_1, SeafloorCavern_Room1_Text_Grunt1Intro, SeafloorCavern_Room1_Text_Grunt1Defeat
	msgbox SeafloorCavern_Room1_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room1_EventScript_Grunt2::
	trainerbattle_single TRAINER_GRUNT_SEAFLOOR_CAVERN_2, SeafloorCavern_Room1_Text_Grunt2Intro, SeafloorCavern_Room1_Text_Grunt2Defeat
	msgbox SeafloorCavern_Room1_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room1_Text_Grunt1Intro:
	.string "A kid? Down here? Go home!\n"
	.string "This is no place for you!$"

SeafloorCavern_Room1_Text_Grunt1Defeat:
	.string "I want to go home…\p"
	.string "I joined to save the reef. How did\n"
	.string "I end up down here?$"

SeafloorCavern_Room1_Text_Grunt1PostBattle:
	.string "Half the crew didn't come back\n"
	.string "after LILYCOVE.\p"
	.string "The ones who stayed are the ones\n"
	.string "who didn't have anywhere else.$"

SeafloorCavern_Room1_Text_Grunt2Intro:
	.string "That submarine is so cramped.\n"
	.string "I'm sore everywhere!$"

SeafloorCavern_Room1_Text_Grunt2Defeat:
	.string "Sore AND beaten…$"

SeafloorCavern_Room1_Text_Grunt2PostBattle:
	.string "That submarine we took is brutal\n"
	.string "to ride. Way too tight in there!$"

