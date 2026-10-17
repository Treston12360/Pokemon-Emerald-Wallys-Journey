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
	.string "How did you manage to get here without\n"
	.string "a submarine?\l"
	.string "What an impressive child!\p"
	.string "But… It won't do to have you\n"
	.string "meddling about here.\p"
	.string "And, I do want payback for what\n"
	.string "happened at the WEATHER INSTITUTE…\p"
	.string "I'm going to give you a little taste\n"
	.string "of pain! Resign yourself to it!$"

SeafloorCavern_Room3_Text_ShellyDefeat:
	.string "Ahahahaha!\p"
	.string "Ouch!$"

SeafloorCavern_Room3_Text_ShellyPostBattle:
	.string "You're strong. Annoyingly strong.\p"
	.string "But listen to me. If this goes\n"
	.string "wrong, it won't be MAGMA's fault or\l"
	.string "yours. It'll be ours.\p"
	.string "…Forget I said that. Move.$"

SeafloorCavern_Room3_Text_Grunt5Intro:
	.string "For our dream to become real, we need\n"
	.string "the power of POKéMON.\p"
	.string "But meddlers like you use the power of\n"
	.string "POKéMON to mess with us even at\l"
	.string "a place like this!\p"
	.string "Life just doesn't work the way we\n"
	.string "need it to!$"

SeafloorCavern_Room3_Text_Grunt5Defeat:
	.string "Gwah!$"

SeafloorCavern_Room3_Text_Grunt5PostBattle:
	.string "We don't question the BOSS.\n"
	.string "That's the rule.\p"
	.string "But here you are, one kid, walking\n"
	.string "straight at him.\p"
	.string "Maybe someone should have\n"
	.string "questioned him a long time ago.$"

