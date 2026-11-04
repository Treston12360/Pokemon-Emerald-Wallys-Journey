OldaleTown_House2_MapScripts::
	.byte 0

OldaleTown_House2_EventScript_Woman::
	msgbox OldaleTown_House2_Text_PokemonLevelUp, MSGBOX_NPC
	end

OldaleTown_House2_EventScript_Man::
	msgbox OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon, MSGBOX_NPC
	end

OldaleTown_House2_Text_PokemonLevelUp:
	.string "Pokémon grow stronger through\n"
	.string "battle. So do Trainers.\p"
	.string "Losing counts too. Maybe more.$"

OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon:
	.string "The stronger your Pokémon get, the\n"
	.string "farther you can travel.\p"
	.string "My grandson went all the way to\n"
	.string "Ever Grande once. Came back a\l"
	.string "different person.$"

