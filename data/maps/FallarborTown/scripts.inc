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
	.string "I ran a produce stand here for\n"
	.string "thirty years. The ash closed it.\p"
	.string "My lungs aren't what they were, so\n"
	.string "I can't take farm work elsewhere.\p"
	.string "No work, no money to leave. No\n"
	.string "leaving, more ash in my lungs.\p"
	.string "It's a circle. Hard to step out of\n"
	.string "one when you're this tired.$"

FallarborTown_Text_MyPreciousAzurill:
	.string "This is my AZURILL!\p"
	.string "We had a berry farm. The ash from\n"
	.string "MT. CHIMNEY buried it. Nothing\l"
	.string "grows there now.\p"
	.string "Dad coughs all night, so he can't\n"
	.string "work the hours he used to.\p"
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
	.string "“A farm town that won't give up.”$"

FallarborTown_Text_MoveTutorSign:
	.string "MOVE TUTOR'S HOUSE\n"
	.string "“New moves taught to POKéMON.”$"

FallarborTown_Text_MyPreciousAzurill_W0:
	.string "TEAM MAGMA ran off the mountain.\p"
	.string "The ash still falls, though. Dad\n"
	.string "says we'd move if we could.\p"
	.string "But a house anywhere else costs\n"
	.string "more than our farm ever made.\p"
	.string "My neighbor's son was in MAGMA. He\n"
	.string "came home ashamed.\p"
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

@ Wally's Journey: trades 1,000 steps of ash (Soot Sack) for a Toxic Orb, then
@ another 1,000 for a Flame Orb. Each once.
	.set ORB_ASH_PRICE, 1000

FallarborTown_EventScript_AshChemist::
	lock
	faceplayer
	goto_if_unset FLAG_RECEIVED_WJ_TOXIC_ORB, FallarborTown_EventScript_AshChemistStart
	goto_if_set FLAG_RECEIVED_WJ_FLAME_ORB, FallarborTown_EventScript_AshChemistDone
FallarborTown_EventScript_AshChemistStart::
	msgbox FallarborTown_Text_AshChemistIntro, MSGBOX_DEFAULT
	checkitem ITEM_SOOT_SACK
	goto_if_eq VAR_RESULT, FALSE, FallarborTown_EventScript_AshChemistNoSack
	goto_if_lt VAR_ASH_GATHER_COUNT, ORB_ASH_PRICE, FallarborTown_EventScript_AshChemistNotEnough
	goto_if_set FLAG_RECEIVED_WJ_TOXIC_ORB, FallarborTown_EventScript_AshChemistFlame
	msgbox FallarborTown_Text_AshChemistOfferToxic, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FallarborTown_EventScript_AshChemistDeclined
	giveitem ITEM_TOXIC_ORB
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	subvar VAR_ASH_GATHER_COUNT, ORB_ASH_PRICE
	setflag FLAG_RECEIVED_WJ_TOXIC_ORB
	msgbox FallarborTown_Text_AshChemistExplainToxic, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_AshChemistFlame::
	msgbox FallarborTown_Text_AshChemistOfferFlame, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FallarborTown_EventScript_AshChemistDeclined
	giveitem ITEM_FLAME_ORB
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	subvar VAR_ASH_GATHER_COUNT, ORB_ASH_PRICE
	setflag FLAG_RECEIVED_WJ_FLAME_ORB
	msgbox FallarborTown_Text_AshChemistExplainFlame, MSGBOX_DEFAULT
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
	.string "I collect the ash that falls on\n"
	.string "FALLARBOR.\p"
	.string "It killed our farms, and it makes\n"
	.string "people sick. I want to know why.\p"
	.string "If I learn what's in it, maybe I can\n"
	.string "find a cure… and a way to make crops\l"
	.string "grow in ash soil again.\p"
	.string "The minerals I pull out on the way\n"
	.string "bind into orbs. One poisons whoever\l"
	.string "holds it. The other burns them.$"

FallarborTown_Text_AshChemistNoSack:
	.string "Bring me 1,000 steps' worth of ash\n"
	.string "for each orb. All of it helps the\l"
	.string "research.\p"
	.string "The GLASS WORKSHOP on ROUTE 113\n"
	.string "gives out SOOT SACKS for it.$"

FallarborTown_Text_AshChemistNotEnough:
	.string "Your SOOT SACK holds {STR_VAR_1} steps\n"
	.string "of ash. I need 1,000 for an orb.$"

FallarborTown_Text_AshChemistOfferToxic:
	.string "That's enough ash. Trade 1,000\n"
	.string "steps of it for a TOXIC ORB?$"

FallarborTown_Text_AshChemistOfferFlame:
	.string "Enough ash for the other one.\n"
	.string "Trade 1,000 steps for a FLAME ORB?$"

FallarborTown_Text_AshChemistExplainToxic:
	.string "A POKéMON holding that orb is badly\n"
	.string "poisoned at the end of a turn.\p"
	.string "That sounds cruel, but some\n"
	.string "POKéMON get stronger when poisoned.\p"
	.string "Bring me another 1,000 steps of ash\n"
	.string "and I'll make a FLAME ORB.$"

FallarborTown_Text_AshChemistExplainFlame:
	.string "A POKéMON holding that orb is\n"
	.string "burned at the end of a turn.\p"
	.string "Some POKéMON hit harder when\n"
	.string "they're burned. Use it well.$"

FallarborTown_Text_AshChemistDeclined:
	.string "Come back if you change your mind.$"

FallarborTown_Text_AshChemistDone:
	.string "Those were the only two orbs I\n"
	.string "could make. The rest of the ash goes\l"
	.string "to the research.\p"
	.string "I'm growing seedlings in ash soil\n"
	.string "now. Most die. A few don't.\p"
	.string "That's a start.$"
