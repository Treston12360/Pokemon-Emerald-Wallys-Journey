OldaleTown_House2_MapScripts::
	.byte 0

OldaleTown_House2_EventScript_Woman::
	msgbox OldaleTown_House2_Text_PokemonLevelUp, MSGBOX_NPC
	end

OldaleTown_House2_EventScript_Man::
	msgbox OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon, MSGBOX_NPC
	end

OldaleTown_House2_Text_PokemonLevelUp:
	.string "POKéMON grow stronger through\n"
	.string "battle. So do TRAINERS.\p"
	.string "Losing counts too. Maybe more.$"

OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon:
	.string "The stronger your POKéMON get, the\n"
	.string "farther you can travel.\p"
	.string "My grandson went all the way to\n"
	.string "EVER GRANDE once. Came back a\l"
	.string "different person.$"

