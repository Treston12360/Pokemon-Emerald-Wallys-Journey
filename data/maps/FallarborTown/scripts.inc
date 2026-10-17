FallarborTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FallarborTown_OnTransition
	.byte 0

FallarborTown_OnTransition:
	setflag FLAG_VISITED_FALLARBOR_TOWN
	setvar VAR_CONTEST_HALL_STATE, 0
	clearflag FLAG_CONTEST_SKETCH_CREATED
	end

FallarborTown_EventScript_ExpertM::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, FallarborTown_EventScript_ExpertMNormal
	msgbox FallarborTown_Text_ShadyCharactersCozmosHome, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_ExpertMNormal::
	goto_if_set FLAG_SYS_GAME_CLEAR, FallarborTown_EventScript_ExpertMChampion
	msgbox FallarborTown_Text_RegionKnownForMeteors, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_ExpertMChampion::
	msgbox FallarborTown_Text_MeteorResearchAfterChampion, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_Girl::
	goto_if_set FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, FallarborTown_Text_MyPreciousAzurill_W0_Script
	msgbox FallarborTown_Text_MyPreciousAzurill, MSGBOX_NPC
	end

FallarborTown_Text_MyPreciousAzurill_W0_Script:
	msgbox FallarborTown_Text_MyPreciousAzurill_W0, MSGBOX_NPC
	end

FallarborTown_EventScript_Gentleman::
	goto_if_set FLAG_SYS_GAME_CLEAR, FallarborTown_EventScript_GentlemanChampion
	msgbox FallarborTown_Text_HaveYouChallengedFlannery, MSGBOX_NPC
	end

FallarborTown_EventScript_GentlemanChampion::
	msgbox FallarborTown_Text_FlanneryAfterChampion, MSGBOX_NPC
	end

FallarborTown_EventScript_Azurill::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_AZURILL, CRY_MODE_NORMAL
	msgbox FallarborTown_Text_Azurill, MSGBOX_DEFAULT
	waitmoncry
	release
	end

FallarborTown_EventScript_BattleTentSign::
	msgbox FallarborTown_Text_BattleTentSign, MSGBOX_SIGN
	end

FallarborTown_EventScript_TownSign::
	msgbox FallarborTown_Text_TownSign, MSGBOX_SIGN
	end

FallarborTown_EventScript_MoveTutorSign::
	msgbox FallarborTown_Text_MoveTutorSign, MSGBOX_SIGN
	end

FallarborTown_Text_ShadyCharactersCozmosHome:
	.string "Something's wrong.\p"
	.string "Shady people in red have been going\n"
	.string "in and out of PROF. COZMO's house.\l"
	.string "They said they were from a housing\l"
	.string "charity.$"

FallarborTown_Text_RegionKnownForMeteors:
	.string "This region's known for meteors.\p"
	.string "METEOR FALLS was carved by one, they\n"
	.string "say. Fire from the sky made this\l"
	.string "whole valley.$"

FallarborTown_Text_MyPreciousAzurill:
	.string "This is my AZURILL!\p"
	.string "The ash from MT. CHIMNEY makes our\n"
	.string "crops grow, but it hurts to breathe\l"
	.string "some days.\p"
	.string "TEAM MAGMA says they can fix the\n"
	.string "mountain. Mom says to never trust\l"
	.string "anyone who says “fix.”$"

FallarborTown_Text_Azurill:
	.string "AZURILL: Rooreelooo.$"

FallarborTown_Text_HaveYouChallengedFlannery:
	.string "Have you challenged FLANNERY yet?\p"
	.string "Her grandfather was ELITE FOUR.\n"
	.string "Everyone expected her to be him.\p"
	.string "She's spent a year proving she's\n"
	.string "someone else. I respect that.$"

FallarborTown_Text_BattleTentSign:
	.string "BATTLE TENT FALLARBOR SITE\n"
	.string "“May the Greatest Teams Gather!”$"

FallarborTown_Text_TownSign:
	.string "FALLARBOR TOWN\n"
	.string "“A farm community with small gardens.”$"

FallarborTown_Text_MoveTutorSign:
	.string "MOVE TUTOR'S HOUSE\n"
	.string "“New moves taught to POKéMON.”$"

FallarborTown_Text_MyPreciousAzurill_W0:
	.string "TEAM MAGMA ran off the mountain.\p"
	.string "My neighbor's son was one of them.\n"
	.string "He came home ashamed.\p"
	.string "His mom made him dinner anyway.\n"
	.string "That's FALLARBOR.$"

FallarborTown_Text_MeteorResearchAfterChampion:
	.string "PROF. COZMO is back at work.\n"
	.string "This time he shares his notes\n"
	.string "with ROXANNE in RUSTBORO.\p"
	.string "He says the meteorite wasn't\n"
	.string "dangerous because it fell.\p"
	.string "It became dangerous when\n"
	.string "people thought they knew\n"
	.string "exactly what it could do.$"

FallarborTown_Text_FlanneryAfterChampion:
	.string "FLANNERY came through town\n"
	.string "with supplies after the storm.\p"
	.string "Someone called her the next\n"
	.string "great LEADER in her family.\p"
	.string "She laughed. 'I'm already\n"
	.string "the first one who's me.'\p"
	.string "Her grandfather would've\n"
	.string "liked that answer.$"
