MossdeepCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MossdeepCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

MossdeepCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_MOSSDEEP_CITY
	end

MossdeepCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_MOSSDEEP_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

MossdeepCity_PokemonCenter_1F_EventScript_Woman::
	goto_if_set FLAG_DEFEATED_MOSSDEEP_GYM, MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable_W0_Script
	msgbox MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable, MSGBOX_NPC
	end

MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable_W0_Script:
	msgbox MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable_W0, MSGBOX_NPC
	end

MossdeepCity_PokemonCenter_1F_EventScript_Girl::
	msgbox MossdeepCity_PokemonCenter_1F_Text_AbilitiesMightChangeMoves, MSGBOX_NPC
	end

MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable:
	.string "TATE and LIZA finish each\n"
	.string "other's sentences.\p"
	.string "It looks like mind reading.\n"
	.string "But they've trained together\n"
	.string "since they were little.\p"
	.string "I guess knowing someone\n"
	.string "that well is its own power.$"

MossdeepCity_PokemonCenter_1F_Text_AbilitiesMightChangeMoves:
	.string "A move can be strong against\n"
	.string "a POKéMON's type and still fail.\p"
	.string "An ABILITY can change what\n"
	.string "happens in battle.\p"
	.string "Knowing your opponent is\n"
	.string "more than memorizing a chart.$"

MossdeepCity_PokemonCenter_1F_Text_GymLeaderDuoFormidable_W0:
	.string "You beat TATE and LIZA?\n"
	.string "They've been studying the\n"
	.string "battle together ever since.\p"
	.string "They don't blame each other.\n"
	.string "They want to know what\n"
	.string "you saw that they missed.$"
