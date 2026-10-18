SeafloorCavern_Room3_MapScripts::
	.byte 0

SeafloorCavern_Room3_EventScript_Shelly::
	trainerbattle_single TRAINER_SHELLY_SEAFLOOR_CAVERN, SeafloorCavern_Room3_Text_ShellyIntro, SeafloorCavern_Room3_Text_ShellyDefeat
	msgbox SeafloorCavern_Room3_Text_ShellyPostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room3_EventScript_Grunt5::
	trainerbattle_single TRAINER_GRUNT_SEAFLOOR_CAVERN_5, SeafloorCavern_Room3_Text_Grunt5Intro, SeafloorCavern_Room3_Text_Grunt5Defeat
	msgbox SeafloorCavern_Room3_Text_Grunt5PostBattle, MSGBOX_AUTOCLOSE
	end

SeafloorCavern_Room3_Text_ShellyIntro:
	.string "Ahahahaha!\p"
	.string "You got here without a submarine?\n"
	.string "How impressive!\p"
	.string "But I can't have you meddling.\p"
	.string "And I owe you for the WEATHER\n"
	.string "INSTITUTE…\p"
	.string "Time for a little taste of pain!$"

SeafloorCavern_Room3_Text_ShellyDefeat:
	.string "Ahahahaha!\p"
	.string "Ow!$"

SeafloorCavern_Room3_Text_ShellyPostBattle:
	.string "You're strong. Annoyingly strong.\p"
	.string "But listen to me. If this goes\n"
	.string "wrong, it won't be MAGMA's fault or\l"
	.string "yours. It'll be ours.\p"
	.string "…Forget I said that. Move.$"

SeafloorCavern_Room3_Text_Grunt5Intro:
	.string "We need POKéMON power to make\n"
	.string "our dream real.\p"
	.string "And meddlers like you use the same\n"
	.string "power to stop us!\p"
	.string "We tried petitions first, you know.\n"
	.string "Nobody listened.$"

SeafloorCavern_Room3_Text_Grunt5Defeat:
	.string "Gwah! Even down here,\n"
	.string "it doesn't work out…$"

SeafloorCavern_Room3_Text_Grunt5PostBattle:
	.string "We don't question the BOSS.\n"
	.string "That's the rule.\p"
	.string "But here you are, one kid, walking\n"
	.string "straight at him.\p"
	.string "Maybe someone should have\n"
	.string "questioned him a long time ago.$"

