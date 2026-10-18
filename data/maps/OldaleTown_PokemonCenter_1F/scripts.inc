OldaleTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, OldaleTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

OldaleTown_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_OLDALE_TOWN
	call Common_EventScript_UpdateBrineyLocation
	end

OldaleTown_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_OLDALE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

OldaleTown_PokemonCenter_1F_EventScript_Gentleman::
	msgbox OldaleTown_PokemonCenter_1F_Text_TrainersCanUsePC, MSGBOX_NPC
	end

OldaleTown_PokemonCenter_1F_EventScript_Boy::
	msgbox OldaleTown_PokemonCenter_1F_Text_PokemonCentersAreGreat, MSGBOX_NPC
	end

OldaleTown_PokemonCenter_1F_EventScript_Girl::
	lock
	faceplayer
	goto_if_set FLAG_SYS_POKEDEX_GET, OldaleTown_PokemonCenter_1F_EventScript_WirelessClubAvailable
	msgbox OldaleTown_PokemonCenter_1F_Text_WirelessClubNotAvailable, MSGBOX_DEFAULT
	release
	end

OldaleTown_PokemonCenter_1F_EventScript_WirelessClubAvailable::
	msgbox OldaleTown_PokemonCenter_1F_Text_TradedInWirelessClub, MSGBOX_DEFAULT
	release
	end

OldaleTown_PokemonCenter_1F_Text_TrainersCanUsePC:
	.string "PROF. BIRCH and NORMAN go way back.\p"
	.string "BIRCH used to drag him out to the\n"
	.string "field. Said he'd never seen a\l"
	.string "TRAINER with that kind of focus.\p"
	.string "BIRCH blames himself for how little\n"
	.string "NORMAN gets home.\p"
	.string "NORMAN just shakes his head. Says\n"
	.string "nobody pushed him but himself.$"

OldaleTown_PokemonCenter_1F_Text_PokemonCentersAreGreat:
	.string "POKéMON CENTERS are free for every\n"
	.string "TRAINER. The LEAGUE pays for it.\p"
	.string "Rich kid, poor kid, same care. I\n"
	.string "think that's the best thing about\l"
	.string "HOENN.$"

OldaleTown_PokemonCenter_1F_Text_WirelessClubNotAvailable:
	.string "The POKéMON WIRELESS CLUB on\n"
	.string "the second floor was built recently.\p"
	.string "But they say they're still making\n"
	.string "adjustments.$"

OldaleTown_PokemonCenter_1F_Text_TradedInWirelessClub:
	.string "OLDALE and LITTLEROOT used to be\n"
	.string "full of young families.\p"
	.string "Then the jobs moved to the cities,\n"
	.string "and the young families followed.\p"
	.string "Now new people are trickling back.\n"
	.string "Because it's all they can afford.$"

