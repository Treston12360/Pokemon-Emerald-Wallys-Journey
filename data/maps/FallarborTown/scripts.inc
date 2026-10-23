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
	.string "This time he shares his notes\l"
	.string "with ROXANNE in RUSTBORO.\p"
	.string "He says the meteorite wasn't\n"
	.string "dangerous because it fell.\p"
	.string "It became dangerous when\n"
	.string "people thought they knew\l"
	.string "exactly what it could do.$"

FallarborTown_Text_FlanneryAfterChampion:
	.string "FLANNERY came through town with\n"
	.string "supplies after the storm.\p"
	.string "Someone called her the next great\n"
	.string "LEADER in her family.\p"
	.string "She laughed. “I'm already the first\n"
	.string "one who's me.”\p"
	.string "Her grandfather loved that answer.$"

@ Wally's Journey: trades 1,000 steps of ash (Soot Sack) for the Toxic Orb, once.
	.set TOXIC_ORB_ASH_PRICE, 1000

FallarborTown_EventScript_AshChemist::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_WJ_TOXIC_ORB, FallarborTown_EventScript_AshChemistDone
	msgbox FallarborTown_Text_AshChemistIntro, MSGBOX_DEFAULT
	checkitem ITEM_SOOT_SACK
	goto_if_eq VAR_RESULT, FALSE, FallarborTown_EventScript_AshChemistNoSack
	goto_if_lt VAR_ASH_GATHER_COUNT, TOXIC_ORB_ASH_PRICE, FallarborTown_EventScript_AshChemistNotEnough
	msgbox FallarborTown_Text_AshChemistOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FallarborTown_EventScript_AshChemistDeclined
	giveitem ITEM_TOXIC_ORB
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	subvar VAR_ASH_GATHER_COUNT, TOXIC_ORB_ASH_PRICE
	setflag FLAG_RECEIVED_WJ_TOXIC_ORB
	msgbox FallarborTown_Text_AshChemistExplain, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_AshChemistNoSack::
	msgbox FallarborTown_Text_AshChemistNoSack, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_AshChemistNotEnough::
	buffernumberstring STR_VAR_1, VAR_ASH_GATHER_COUNT
	msgbox FallarborTown_Text_AshChemistNotEnough, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_AshChemistDeclined::
	msgbox FallarborTown_Text_AshChemistDeclined, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_AshChemistDone::
	msgbox FallarborTown_Text_AshChemistDone, MSGBOX_DEFAULT
	release
	end

FallarborTown_Text_AshChemistIntro:
	.string "I study the ash that falls on\n"
	.string "FALLARBOR. It's full of minerals\l"
	.string "from deep inside MT. CHIMNEY.\p"
	.string "I can bind them into an orb that\n"
	.string "poisons whoever holds it.$"

FallarborTown_Text_AshChemistNoSack:
	.string "Bring me 1,000 steps' worth of ash\n"
	.string "and the orb is yours.\p"
	.string "The GLASS WORKSHOP on ROUTE 113\n"
	.string "gives out SOOT SACKS for it.$"

FallarborTown_Text_AshChemistNotEnough:
	.string "Your SOOT SACK holds {STR_VAR_1} steps\n"
	.string "of ash. I need 1,000 for the orb.$"

FallarborTown_Text_AshChemistOffer:
	.string "That's enough ash. Trade 1,000\n"
	.string "steps of it for a TOXIC ORB?$"

FallarborTown_Text_AshChemistExplain:
	.string "A POKéMON holding that orb is badly\n"
	.string "poisoned at the end of a turn.\p"
	.string "That sounds cruel, but some\n"
	.string "POKéMON get stronger when poisoned.$"

FallarborTown_Text_AshChemistDeclined:
	.string "Come back if you change your mind.$"

FallarborTown_Text_AshChemistDone:
	.string "That was all the mineral I could\n"
	.string "pull from the ash. One orb only.$"
