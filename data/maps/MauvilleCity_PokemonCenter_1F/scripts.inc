MauvilleCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MauvilleCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

MauvilleCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_MAUVILLE_CITY
	call Common_EventScript_UpdateBrineyLocation
	goto MauvilleCity_PokemonCenter_1F_EventScript_SetMauvilleOldManGfx
	end

MauvilleCity_PokemonCenter_1F_EventScript_SetMauvilleOldManGfx::
	special SetMauvilleOldManObjEventGfx
	end

MauvilleCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_MAUVILLE_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

MauvilleCity_PokemonCenter_1F_EventScript_Woman1::
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings_W0_Script
	msgbox MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings, MSGBOX_NPC
	end

MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings_W0_Script:
	msgbox MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings_W0, MSGBOX_NPC
	end

MauvilleCity_PokemonCenter_1F_EventScript_Woman2::
	msgbox MauvilleCity_PokemonCenter_1F_Text_MyDataUpdatedFromRecordCorner, MSGBOX_NPC
	end

MauvilleCity_PokemonCenter_1F_EventScript_Youngster::
	msgbox MauvilleCity_PokemonCenter_1F_Text_RecordCornerSoundsFun, MSGBOX_NPC
	end

MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings:
	.string "That man over there, he says weird\n"
	.string "things!\p"
	.string "He's funny in a weird way.\n"
	.string "I doubt I'll forget about him!$"

MauvilleCity_PokemonCenter_1F_Text_MyDataUpdatedFromRecordCorner:
	.string "I've been on the housing list for two\n"
	.string "years.\p"
	.string "A LAND INITIATIVE fella came by. Said\n"
	.string "they'll build new towns on the old\l"
	.string "lava fields.\p"
	.string "Red hood, nice smile. My mother says\n"
	.string "not to go. I'm thinking about it.$"

MauvilleCity_PokemonCenter_1F_Text_RecordCornerSoundsFun:
	.string "I heard MAXIE speak once, years ago.\n"
	.string "Town hall in FALLARBOR.\p"
	.string "He had blueprints. Real plans for\n"
	.string "homes and farms. People cried.\p"
	.string "That's the man they call a criminal\n"
	.string "now. Something happened to him.$"

MauvilleCity_PokemonCenter_1F_Text_ManOverThereSaysWeirdThings_W0:
	.string "WATTSON lost to a kid and laughed so\n"
	.string "hard the lights flickered.\p"
	.string "That was you? Wahahaha!$"
