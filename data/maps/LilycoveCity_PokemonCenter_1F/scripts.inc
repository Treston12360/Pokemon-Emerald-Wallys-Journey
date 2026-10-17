LilycoveCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LilycoveCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

LilycoveCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_LILYCOVE_CITY
	goto LilycoveCity_PokemonCenter_1F_EventScript_SetLilycoveLadyGfx
	end

@ SetLilycoveLadyGfx returns TRUE if its the Contest Lady
LilycoveCity_PokemonCenter_1F_EventScript_SetLilycoveLadyGfx::
	special SetLilycoveLadyGfx
	goto_if_eq VAR_RESULT, FALSE, LilycoveCity_PokemonCenter_1F_EventScript_HideContestLadyMon
	goto_if_eq VAR_RESULT, TRUE, LilycoveCity_PokemonCenter_1F_EventScript_ShowContestLadyMon
	end

LilycoveCity_PokemonCenter_1F_EventScript_HideContestLadyMon::
	setflag FLAG_HIDE_LILYCOVE_POKEMON_CENTER_CONTEST_LADY_MON
	end

LilycoveCity_PokemonCenter_1F_EventScript_ShowContestLadyMon::
	clearflag FLAG_HIDE_LILYCOVE_POKEMON_CENTER_CONTEST_LADY_MON
	end

LilycoveCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_LILYCOVE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

LilycoveCity_PokemonCenter_1F_EventScript_Boy::
	goto_if_set FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon_W0_Script
	msgbox LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon, MSGBOX_NPC
	end

LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon_W0_Script:
	msgbox LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon_W0, MSGBOX_NPC
	end

LilycoveCity_PokemonCenter_1F_EventScript_Maniac::
	lock
	faceplayer
	goto_if_set FLAG_BADGE07_GET, LilycoveCity_PokemonCenter_1F_EventScript_ManiacBadTeamGone
	msgbox LilycoveCity_PokemonCenter_1F_Text_HeardAboutRottenScoundrels, MSGBOX_DEFAULT
	release
	end

LilycoveCity_PokemonCenter_1F_EventScript_ManiacBadTeamGone::
	msgbox LilycoveCity_PokemonCenter_1F_Text_HaventSeenRottenScoundrels, MSGBOX_DEFAULT
	release
	end

LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon:
	.string "I wonder how many kinds of POKéMON\n"
	.string "there are in the world.\p"
	.string "It'd be great to cross seas and\n"
	.string "trade POKéMON with people far away.$"

LilycoveCity_PokemonCenter_1F_Text_HeardAboutRottenScoundrels:
	.string "A traveler told me people\n"
	.string "were stealing research\n"
	.string "equipment and POKéMON.\p"
	.string "I used to think these teams\n"
	.string "were just arguing about\n"
	.string "how to protect HOENN.\p"
	.string "Now I'm not so sure.$"

LilycoveCity_PokemonCenter_1F_Text_HaventSeenRottenScoundrels:
	.string "The people stealing equipment\n"
	.string "haven't been seen lately.\p"
	.string "That doesn't mean the problem\n"
	.string "went away.\p"
	.string "I hope whoever was hurt gets\n"
	.string "their POKéMON back.$"

LilycoveCity_PokemonCenter_1F_Text_HowManyKindsOfPokemon_W0:
	.string "The cave in the cove was TEAM AQUA's\n"
	.string "base the whole time.\p"
	.string "Right under our noses. Makes you\n"
	.string "wonder what else we didn't see.$"
