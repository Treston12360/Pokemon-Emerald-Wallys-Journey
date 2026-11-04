RustboroCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RustboroCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

RustboroCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_RUSTBORO_CITY
	call Common_EventScript_UpdateBrineyLocation
	end

RustboroCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_RUSTBORO_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_Man::
	msgbox RustboroCity_PokemonCenter_1F_Text_PokemonHavePersonalities, MSGBOX_NPC
	end

RustboroCity_PokemonCenter_1F_EventScript_Boy::
	msgbox RustboroCity_PokemonCenter_1F_Text_MaleAndFemalePokemon, MSGBOX_NPC
	end

RustboroCity_PokemonCenter_1F_EventScript_Girl::
	msgbox RustboroCity_PokemonCenter_1F_Text_HMCutNextDoor, MSGBOX_NPC
	end

RustboroCity_PokemonCenter_1F_Text_PokemonHavePersonalities:
	.string "My Pokémon is Naive. My friend's is\n"
	.string "Jolly.\p"
	.string "Natures change how they grow. Just\n"
	.string "like people.$"

RustboroCity_PokemonCenter_1F_Text_MaleAndFemalePokemon:
	.string "Male and female Pokémon…\p"
	.string "Prof. Birch says the differences are\n"
	.string "real, but subtle. Like a lot of\l"
	.string "things in life.$"

RustboroCity_PokemonCenter_1F_Text_HMCutNextDoor:
	.string "Roxanne gave me HM Cut along with\n"
	.string "my Stone Badge.\p"
	.string "She said the Badge is the license.\n"
	.string "The HM is just proof you earned it.$"

