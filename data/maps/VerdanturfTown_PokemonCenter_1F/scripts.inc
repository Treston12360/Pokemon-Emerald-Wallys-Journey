VerdanturfTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VerdanturfTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

VerdanturfTown_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_VERDANTURF_TOWN
	call Common_EventScript_UpdateBrineyLocation
	end

VerdanturfTown_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_VERDANTURF_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

VerdanturfTown_PokemonCenter_1F_EventScript_Gentleman::
	msgbox VerdanturfTown_PokemonCenter_1F_Text_FaithInYourPokemon, MSGBOX_NPC
	end

VerdanturfTown_PokemonCenter_1F_EventScript_ExpertM::
	msgbox VerdanturfTown_PokemonCenter_1F_Text_VisitForBattleTent, MSGBOX_NPC
	end

VerdanturfTown_PokemonCenter_1F_Text_FaithInYourPokemon:
	.string "The tunnel to RUSTBORO was DEVON's\n"
	.string "idea. Faster shipping.\p"
	.string "They stopped digging when the\n"
	.string "WHISMUR started going deaf from\l"
	.string "the drills.\p"
	.string "Took a lot of complaining. But they\n"
	.string "did stop. Give them that.$"

VerdanturfTown_PokemonCenter_1F_Text_VisitForBattleTent:
	.string "Why would anyone visit VERDANTURF?\p"
	.string "The BATTLE TENT, or the clean air.\p"
	.string "Or maybe to see the sick kids get\n"
	.string "better. That's the real reason this\l"
	.string "town matters.$"

