FortreeCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FortreeCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

FortreeCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_FORTREE_CITY
	end

FortreeCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_FORTREE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

FortreeCity_PokemonCenter_1F_EventScript_Gentleman::
	msgbox FortreeCity_PokemonCenter_1F_Text_GoToSafariZone, MSGBOX_NPC
	end

FortreeCity_PokemonCenter_1F_EventScript_Man::
	msgbox FortreeCity_PokemonCenter_1F_Text_RecordCornerIsNeat, MSGBOX_NPC
	end

FortreeCity_PokemonCenter_1F_EventScript_Boy::
	msgbox FortreeCity_PokemonCenter_1F_Text_DoYouKnowAboutPokenav, MSGBOX_NPC
	end

FortreeCity_PokemonCenter_1F_Text_GoToSafariZone:
	.string "Listen, kid, are you working\n"
	.string "on a Pokédex?\p"
	.string "Hmm… Go to the Safari Zone.\n"
	.string "That's my suggestion.$"

FortreeCity_PokemonCenter_1F_Text_RecordCornerIsNeat:
	.string "The oldest song in Fortree is about\n"
	.string "“the one above the sky.”\p"
	.string "When the land and sea fought, it came\n"
	.string "down and they both went quiet.\p"
	.string "Grandma sings it when there's a\n"
	.string "storm. I used to think it was\l"
	.string "just a lullaby.$"

FortreeCity_PokemonCenter_1F_Text_DoYouKnowAboutPokenav:
	.string "Oh, wow, you have a PokéNav!\n"
	.string "And it's just like mine!\p"
	.string "Do you know about PokéNav's\n"
	.string "Match Call system?\p"
	.string "Access it, and you can chat with\n"
	.string "registered Trainers anytime.\p"
	.string "It also shows you which Trainers\n"
	.string "want a rematch with you.\p"
	.string "It's really nifty! Those Devon guys\n"
	.string "sure know what they're doing!$"

