MauvilleCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MauvilleCity_Gym_OnLoad
	.byte 0

MauvilleCity_Gym_OnLoad:
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_DeactivatePuzzle
	switch VAR_MAUVILLE_GYM_STATE
	case 0, MauvilleCity_Gym_EventScript_UpdateBarriers
	case 1, MauvilleCity_Gym_EventScript_Switch1Pressed
	case 2, MauvilleCity_Gym_EventScript_Switch2Pressed
	case 3, MauvilleCity_Gym_EventScript_Switch3Pressed
	case 4, MauvilleCity_Gym_EventScript_Switch4Pressed
	end

MauvilleCity_Gym_EventScript_UpdateBarriers::
	goto_if_set FLAG_MAUVILLE_GYM_BARRIERS_STATE, MauvilleCity_Gym_EventScript_SetAltBarriers
	end

MauvilleCity_Gym_EventScript_SetAltBarriers::
	setmetatile 3, 11, METATILE_MauvilleGym_RedBeamV1_On, TRUE
	setmetatile 3, 12, METATILE_MauvilleGym_RedBeamV2_On, TRUE
	setmetatile 3, 13, METATILE_MauvilleGym_PoleTop_On, TRUE
	setmetatile 4, 10, METATILE_MauvilleGym_RedBeamH1_On, FALSE
	setmetatile 5, 10, METATILE_MauvilleGym_RedBeamH2_On, FALSE
	setmetatile 4, 11, METATILE_MauvilleGym_RedBeamH3_On, TRUE
	setmetatile 5, 11, METATILE_MauvilleGym_RedBeamH4_On, TRUE
	setmetatile 7, 10, METATILE_MauvilleGym_RedBeamH1_On, FALSE
	setmetatile 8, 10, METATILE_MauvilleGym_RedBeamH2_On, FALSE
	setmetatile 7, 11, METATILE_MauvilleGym_RedBeamH3_On, TRUE
	setmetatile 8, 11, METATILE_MauvilleGym_RedBeamH4_On, TRUE
	setmetatile 4, 13, METATILE_MauvilleGym_GreenBeamH1_Off, FALSE
	setmetatile 5, 13, METATILE_MauvilleGym_GreenBeamH2_Off, FALSE
	setmetatile 4, 14, METATILE_MauvilleGym_GreenBeamH3_Off, FALSE
	setmetatile 5, 14, METATILE_MauvilleGym_GreenBeamH4_Off, FALSE
	setmetatile 1, 10, METATILE_MauvilleGym_GreenBeamH1_Off, FALSE
	setmetatile 2, 10, METATILE_MauvilleGym_GreenBeamH2_Off, FALSE
	setmetatile 1, 11, METATILE_MauvilleGym_GreenBeamH3_Off, FALSE
	setmetatile 2, 11, METATILE_MauvilleGym_GreenBeamH4_Off, FALSE
	setmetatile 6, 8, METATILE_MauvilleGym_PoleBottom_On, TRUE
	setmetatile 6, 9, METATILE_MauvilleGym_FloorTile, FALSE
	setmetatile 6, 10, METATILE_MauvilleGym_PoleTop_Off, FALSE
	setmetatile 4, 6, METATILE_MauvilleGym_GreenBeamH1_Off, FALSE
	setmetatile 5, 6, METATILE_MauvilleGym_GreenBeamH2_Off, FALSE
	setmetatile 4, 7, METATILE_MauvilleGym_GreenBeamH3_Off, FALSE
	setmetatile 5, 7, METATILE_MauvilleGym_GreenBeamH4_Off, FALSE
	end

MauvilleCity_Gym_EventScript_Switch1Pressed::
	setvar VAR_0x8004, 0
	special MauvilleGymPressSwitch
	goto MauvilleCity_Gym_EventScript_UpdateBarriers
	end

MauvilleCity_Gym_EventScript_Switch2Pressed::
	setvar VAR_0x8004, 1
	special MauvilleGymPressSwitch
	goto MauvilleCity_Gym_EventScript_UpdateBarriers
	end

MauvilleCity_Gym_EventScript_Switch3Pressed::
	setvar VAR_0x8004, 2
	special MauvilleGymPressSwitch
	goto MauvilleCity_Gym_EventScript_UpdateBarriers
	end

MauvilleCity_Gym_EventScript_Switch4Pressed::
	setvar VAR_0x8004, 3
	special MauvilleGymPressSwitch
	goto MauvilleCity_Gym_EventScript_UpdateBarriers
	end

MauvilleCity_Gym_EventScript_DeactivatePuzzle::
	special MauvilleGymDeactivatePuzzle
	end

MauvilleCity_Gym_EventScript_Wattson::
	trainerbattle_single TRAINER_WATTSON_1, MauvilleCity_Gym_Text_WattsonIntro, MauvilleCity_Gym_Text_WattsonDefeat, MauvilleCity_Gym_EventScript_WattsonDefeated, NO_MUSIC
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_SHOCK_WAVE, MauvilleCity_Gym_EventScript_GiveShockWave2
	goto MauvilleCity_Gym_EventScript_WattsonRematchOffer

MauvilleCity_Gym_EventScript_WattsonDefeated::
	message MauvilleCity_Gym_Text_ReceivedDynamoBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox MauvilleCity_Gym_Text_ExplainDynamoBadgeTakeThis, MSGBOX_DEFAULT
	setvar VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE, 3
	clearflag FLAG_HIDE_VERDANTURF_TOWN_SCOTT
	setflag FLAG_DEFEATED_MAUVILLE_GYM
	setflag FLAG_BADGE03_GET
	setvar VAR_WALLY_RIVAL_PENDING, 3
	addvar VAR_PETALBURG_GYM_STATE, 1
	call_if_eq VAR_PETALBURG_GYM_STATE, 6, Common_EventScript_ReadyPetalburgGymForBattle
	setvar VAR_0x8008, 3
	call Common_EventScript_SetGymTrainers
	special MauvilleGymDeactivatePuzzle
	special DrawWholeMapView
	playse SE_UNLOCK
	call MauvilleCity_Gym_EventScript_GiveShockWave
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox MauvilleCity_Gym_Text_RegisteredWattson, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_WATTSON_MATCH_CALL
	release
	end

MauvilleCity_Gym_EventScript_GiveShockWave2::
	giveitem ITEM_TM_SHOCK_WAVE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox MauvilleCity_Gym_Text_ExplainShockWave, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_SHOCK_WAVE
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

MauvilleCity_Gym_EventScript_GiveShockWave::
	giveitem ITEM_TM_SHOCK_WAVE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_BagIsFull
	msgbox MauvilleCity_Gym_Text_ExplainShockWave, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_SHOCK_WAVE
	call WallyJourney_EventScript_GiveGymHMs
	return

MauvilleCity_Gym_EventScript_CompletedNewMauville::
	msgbox MauvilleCity_Gym_Text_WattsonGoForthAndEndeavor, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_WattsonRematch::
	goto MauvilleCity_Gym_EventScript_WattsonRematchOffer

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
MauvilleCity_Gym_EventScript_WattsonRematchTier3::
	trainerbattle_rematch_double TRAINER_WATTSON_1, MauvilleCity_Gym_Text_WattsonPreRematch3, MauvilleCity_Gym_Text_WattsonRematchDefeat, MauvilleCity_Gym_Text_WattsonRematchNeedTwoMons
	msgbox MauvilleCity_Gym_Text_WattsonPostRematch, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_WattsonRematchTier4::
	trainerbattle_rematch_double TRAINER_WATTSON_1, MauvilleCity_Gym_Text_WattsonPreRematch4, MauvilleCity_Gym_Text_WattsonRematchDefeatElite, MauvilleCity_Gym_Text_WattsonRematchNeedTwoMons
	msgbox MauvilleCity_Gym_Text_WattsonPostRematchElite, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_WattsonRematchTier5::
	trainerbattle_rematch_double TRAINER_WATTSON_1, MauvilleCity_Gym_Text_WattsonPreRematch5, MauvilleCity_Gym_Text_WattsonRematchDefeatElite, MauvilleCity_Gym_Text_WattsonRematchNeedTwoMons
	msgbox MauvilleCity_Gym_Text_WattsonPostRematchElite, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_Switch1::
	lockall
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_SwitchDoNothing
	goto_if_eq VAR_MAUVILLE_GYM_STATE, 1, MauvilleCity_Gym_EventScript_SwitchDoNothing
	setvar VAR_MAUVILLE_GYM_STATE, 1
	setvar VAR_0x8004, 0
	goto MauvilleCity_Gym_EventScript_PressFloorSwitch
	end

MauvilleCity_Gym_EventScript_Switch2::
	lockall
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_SwitchDoNothing
	goto_if_eq VAR_MAUVILLE_GYM_STATE, 2, MauvilleCity_Gym_EventScript_SwitchDoNothing
	setvar VAR_MAUVILLE_GYM_STATE, 2
	setvar VAR_0x8004, 1
	goto MauvilleCity_Gym_EventScript_PressFloorSwitch
	end

MauvilleCity_Gym_EventScript_Switch3::
	lockall
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_SwitchDoNothing
	goto_if_eq VAR_MAUVILLE_GYM_STATE, 3, MauvilleCity_Gym_EventScript_SwitchDoNothing
	setvar VAR_MAUVILLE_GYM_STATE, 3
	setvar VAR_0x8004, 2
	goto MauvilleCity_Gym_EventScript_PressFloorSwitch
	end

MauvilleCity_Gym_EventScript_Switch4::
	lockall
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_SwitchDoNothing
	goto_if_eq VAR_MAUVILLE_GYM_STATE, 4, MauvilleCity_Gym_EventScript_SwitchDoNothing
	setvar VAR_MAUVILLE_GYM_STATE, 4
	setvar VAR_0x8004, 3
	goto MauvilleCity_Gym_EventScript_PressFloorSwitch
	end

MauvilleCity_Gym_EventScript_PressFloorSwitch::
	special MauvilleGymSetDefaultBarriers
	special MauvilleGymPressSwitch
	special DrawWholeMapView
	playse SE_UNLOCK
	goto_if_set FLAG_MAUVILLE_GYM_BARRIERS_STATE, MauvilleCity_Gym_EventScript_ClearBarriersAltState
	goto_if_unset FLAG_MAUVILLE_GYM_BARRIERS_STATE, MauvilleCity_Gym_EventScript_SetBarriersAltState
	releaseall
	end

MauvilleCity_Gym_EventScript_SwitchDoNothing::
	releaseall
	end

MauvilleCity_Gym_EventScript_SetBarriersAltState::
	setflag FLAG_MAUVILLE_GYM_BARRIERS_STATE
	releaseall
	end

MauvilleCity_Gym_EventScript_ClearBarriersAltState::
	clearflag FLAG_MAUVILLE_GYM_BARRIERS_STATE
	releaseall
	end

MauvilleCity_Gym_EventScript_Kirk::
	trainerbattle_single TRAINER_KIRK, MauvilleCity_Gym_Text_KirkIntro, MauvilleCity_Gym_Text_KirkDefeat
	msgbox MauvilleCity_Gym_Text_KirkPostBattle, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_Shawn::
	trainerbattle_single TRAINER_SHAWN, MauvilleCity_Gym_Text_ShawnIntro, MauvilleCity_Gym_Text_ShawnDefeat
	msgbox MauvilleCity_Gym_Text_ShawnPostBattle, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_Ben::
	trainerbattle_single TRAINER_BEN, MauvilleCity_Gym_Text_BenIntro, MauvilleCity_Gym_Text_BenDefeat
	msgbox MauvilleCity_Gym_Text_BenPostBattle, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_Vivian::
	trainerbattle_single TRAINER_VIVIAN, MauvilleCity_Gym_Text_VivianIntro, MauvilleCity_Gym_Text_VivianDefeat
	msgbox MauvilleCity_Gym_Text_VivianPostBattle, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_Angelo::
	trainerbattle_single TRAINER_ANGELO, MauvilleCity_Gym_Text_AngeloIntro, MauvilleCity_Gym_Text_AngeloDefeat
	msgbox MauvilleCity_Gym_Text_AngeloPostBattle, MSGBOX_AUTOCLOSE
	end

MauvilleCity_Gym_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_MAUVILLE_GYM, MauvilleCity_Gym_EventScript_GymGuidePostVictory
	msgbox MauvilleCity_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_GymGuidePostVictory::
	msgbox MauvilleCity_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE03_GET, MauvilleCity_Gym_EventScript_GymStatueCertified
	goto MauvilleCity_Gym_EventScript_GymStatue
	end

MauvilleCity_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE03_GET, MauvilleCity_Gym_EventScript_GymStatueCertified
	goto MauvilleCity_Gym_EventScript_GymStatue
	end

MauvilleCity_Gym_EventScript_GymStatueCertified::
	msgbox MauvilleCity_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

MauvilleCity_Gym_EventScript_GymStatue::
	msgbox MauvilleCity_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

MauvilleCity_Gym_Text_GymGuideAdvice:
	.string "Hey, how's it going, CHAMPION-\n"
	.string "bound {PLAYER}?\p"
	.string "WATTSON, the LEADER of MAUVILLE\n"
	.string "GYM, uses ELECTRIC-type POKéMON.\p"
	.string "If you challenge him with WATER-type\n"
	.string "POKéMON, he'll zap them! Bzzt!\p"
	.string "And, he's put in switch-controlled\n"
	.string "doors all over his GYM! Eccentric!\p"
	.string "Hey, go for it!$"

MauvilleCity_Gym_Text_GymGuidePostVictory:
	.string "Whoa, you're electrifying!\n"
	.string "You've powered the door open!\p"
	.string "A BADGE is a LEAGUE license, too.\n"
	.string "It lets your POKéMON use HM moves\l"
	.string "outside of battle, legally.$"

MauvilleCity_Gym_Text_KirkIntro:
	.string "I play WATTSON's birthday party every\n"
	.string "year, whoa-yeah!\p"
	.string "He dances worse than anyone alive.\n"
	.string "Now feel the voltage!$"

MauvilleCity_Gym_Text_KirkDefeat:
	.string "Plugged in, amped up…\n"
	.string "and blown out!$"

MauvilleCity_Gym_Text_KirkPostBattle:
	.string "When NEW MAUVILLE shut down, my\n"
	.string "dad lost his job down there.\p"
	.string "WATTSON paid his rent for a year.\n"
	.string "Never told anybody. Whoa-yeah.$"

MauvilleCity_Gym_Text_ShawnIntro:
	.string "WATTSON trained me for free when I\n"
	.string "had nothing.\p"
	.string "Now I train the new kids. I won't\n"
	.string "go down easy!$"

MauvilleCity_Gym_Text_ShawnDefeat:
	.string "Unplugged.\n"
	.string "Totally switched off.$"

MauvilleCity_Gym_Text_ShawnPostBattle:
	.string "WATTSON's been battling since before\n"
	.string "your daddy was born.\p"
	.string "He built half of MAUVILLE's power\n"
	.string "grid himself. The man is a legend.$"

MauvilleCity_Gym_Text_BenIntro:
	.string "WATTSON rewires these barriers every\n"
	.string "month just to mess with us!\p"
	.string "I've memorized this month's. Mostly.$"

MauvilleCity_Gym_Text_BenDefeat:
	.string "The doors confused ME more\n"
	.string "than you!$"

MauvilleCity_Gym_Text_BenPostBattle:
	.string "WATTSON loves his switch traps. Says\n"
	.string "a TRAINER who can't think under\l"
	.string "pressure won't last.$"

MauvilleCity_Gym_Text_VivianIntro:
	.string "I moved here for the job at the\n"
	.string "GAME CORNER.\p"
	.string "Four roommates, one bathroom.\n"
	.string "I need to shock SOMEBODY!$"

MauvilleCity_Gym_Text_VivianDefeat:
	.string "Shocking!\n"
	.string "In the bad way!$"

MauvilleCity_Gym_Text_VivianPostBattle:
	.string "WATTSON founded this city.\p"
	.string "He wanted to roof the whole thing\n"
	.string "and run it on one generator. NEW\l"
	.string "MAUVILLE. It… didn't go well.$"

MauvilleCity_Gym_Text_AngeloIntro:
	.string "WATTSON says I can stay as long as\n"
	.string "I keep the lights polished!\p"
	.string "Ooh, your BADGES are shinier!$"

MauvilleCity_Gym_Text_AngeloDefeat:
	.string "Too bright… I can't see\n"
	.string "straight anymore…$"

MauvilleCity_Gym_Text_AngeloPostBattle:
	.string "WATTSON lets anyone train here for\n"
	.string "free. Even me.\p"
	.string "He says nobody in MAUVILLE gets\n"
	.string "turned away. That's why it's so\l"
	.string "crowded, I think.$"

MauvilleCity_Gym_Text_WattsonIntro:
	.string "Wahahaha! You got through my doors!\p"
	.string "I built this city under one roof so\n"
	.string "nobody would freeze or flood.\p"
	.string "Then I built a power plant to light\n"
	.string "all of HOENN. That one… I locked\l"
	.string "away.\p"
	.string "Now I build traps for TRAINERS.\n"
	.string "Safer hobby!\p"
	.string "I, WATTSON, LEADER of MAUVILLE GYM,\n"
	.string "shall electrify you!$"

MauvilleCity_Gym_Text_WattsonDefeat:
	.string "Wahahaha! Fine, I lost!\p"
	.string "That was the best jolt I've had in\n"
	.string "years! Take this BADGE!$"

MauvilleCity_Gym_Text_ReceivedDynamoBadge:
	.string "{PLAYER} received the DYNAMO BADGE\n"
	.string "from WATTSON.$"

MauvilleCity_Gym_Text_ExplainDynamoBadgeTakeThis:
	.string "With the DYNAMO BADGE, POKéMON can\n"
	.string "use ROCK SMASH out of battle.\p"
	.string "And, it will make your POKéMON a little\n"
	.string "bit faster, too.\p"
	.string "Hmm…\n"
	.string "You should take this, too!$"

MauvilleCity_Gym_Text_ExplainShockWave:
	.string "That TM34 there contains SHOCK WAVE.\p"
	.string "It's a trustworthy move that never\n"
	.string "misses! You can count on it!\p"
	.string "… … … … … …$"

MauvilleCity_Gym_Text_RegisteredWattson:
	.string "Registered GYM LEADER WATTSON\n"
	.string "in the POKéNAV.$"

MauvilleCity_Gym_Text_WattsonPostBattle:
	.string "I swell with optimism, seeing a\n"
	.string "promising young TRAINER like you!\p"
	.string "Wahahaha! That WALLY boy came by\n"
	.string "too, after his uncle stopped\l"
	.string "nagging. Gave me a real jolt!$"

MauvilleCity_Gym_Text_WattsonGoForthAndEndeavor:
	.string "Wahahahah! Go forth and endeavor,\n"
	.string "youngster!$"

MauvilleCity_Gym_Text_GymStatue:
	.string "MAUVILLE CITY POKéMON GYM$"

MauvilleCity_Gym_Text_GymStatueCertified:
	.string "MAUVILLE CITY POKéMON GYM\p"
	.string "WATTSON'S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

MauvilleCity_Gym_Text_WattsonPreRematch:
	.string "WATTSON: Ah-ha! Here at last!\p"
	.string "I rerouted three generators to get\n"
	.string "my team this charged. Wahahaha!\p"
	.string "Don't say I didn't warn you!$"

MauvilleCity_Gym_Text_WattsonRematchDefeat:
	.string "Oof…\n"
	.string "Batteries drained!$"

MauvilleCity_Gym_Text_WattsonPostRematch:
	.string "WATTSON: We'll recharge and try\n"
	.string "again!\p"
	.string "The city still runs on my old wiring,\n"
	.string "you know. It holds. Mostly.\p"
	.string "Come back anytime! Wahahaha!$"

MauvilleCity_Gym_Text_WattsonRematchNeedTwoMons:
	.string "WATTSON: Ah-ha! Here at last!\p"
	.string "Oops! Wait! Only one POKéMON?\p"
	.string "My rematches are two at a time.\n"
	.string "Come back with two or more!$"

MauvilleCity_Gym_Text_WattsonPreRematch3:
	.string "WATTSON: Wahahaha! I rewired the\n"
	.string "whole GYM last week!\p"
	.string "Blew every fuse in MAUVILLE doing it.\n"
	.string "Worth it! Can you take the voltage?$"

MauvilleCity_Gym_Text_WattsonPreRematch4:
	.string "WATTSON: The storms knocked out half\n"
	.string "of HOENN's power.\p"
	.string "My POKéMON kept the hospitals lit for\n"
	.string "three nights straight.\p"
	.string "They've never been tougher.\n"
	.string "Brace yourself!$"

MauvilleCity_Gym_Text_WattsonPreRematch5:
	.string "WATTSON: At my age, a man thinks\n"
	.string "about what he'll leave behind.\p"
	.string "A city. A GYM. And this team, at full\n"
	.string "charge! Wahahaha!\p"
	.string "Let's light up the sky one more time!$"

MauvilleCity_Gym_Text_WattsonRematchDefeatElite:
	.string "Wahaha… Total blackout!$"

MauvilleCity_Gym_Text_WattsonPostRematchElite:
	.string "WATTSON: NEW MAUVILLE was the\n"
	.string "biggest mistake of my life.\p"
	.string "Losing to you isn't. Best thing to\n"
	.string "happen to this GYM in years!$"

@ Wally's Journey: growing Gym Leader rematches (see WallyJourney_ScaleLeaderParty).
MauvilleCity_Gym_EventScript_WattsonRematchOffer::
	setvar VAR_0x8004, 3
	special WallyJourney_SetupLeaderRematch
	switch VAR_RESULT
	case 1, MauvilleCity_Gym_EventScript_WattsonRematchStage1
	case 2, MauvilleCity_Gym_EventScript_WattsonRematchStage2
	case 3, MauvilleCity_Gym_EventScript_WattsonRematchStage3
	case 4, MauvilleCity_Gym_EventScript_WattsonRematchStage4
	goto MauvilleCity_Gym_EventScript_WattsonRematchStage1

MauvilleCity_Gym_EventScript_WattsonRematchStage1::
	msgbox MauvilleCity_Gym_Text_WattsonGrowOffer1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_Gym_EventScript_WattsonRematchDecline
	cleartrainerflag TRAINER_WATTSON_2
	trainerbattle_no_intro TRAINER_WATTSON_2, MauvilleCity_Gym_Text_WattsonGrowDefeat1
	msgbox MauvilleCity_Gym_Text_WattsonGrowAfter1, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_WattsonRematchStage2::
	msgbox MauvilleCity_Gym_Text_WattsonGrowOffer2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_Gym_EventScript_WattsonRematchDecline
	cleartrainerflag TRAINER_WATTSON_2
	trainerbattle_no_intro TRAINER_WATTSON_2, MauvilleCity_Gym_Text_WattsonGrowDefeat2
	msgbox MauvilleCity_Gym_Text_WattsonGrowAfter2, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_WattsonRematchStage3::
	msgbox MauvilleCity_Gym_Text_WattsonGrowOffer3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_Gym_EventScript_WattsonRematchDecline
	cleartrainerflag TRAINER_WATTSON_3
	trainerbattle_double TRAINER_WATTSON_3, MauvilleCity_Gym_Text_WattsonGrowGo, MauvilleCity_Gym_Text_WattsonGrowDefeat3, MauvilleCity_Gym_Text_WattsonGrowNeedTwo
	msgbox MauvilleCity_Gym_Text_WattsonGrowAfter3, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_WattsonRematchStage4::
	msgbox MauvilleCity_Gym_Text_WattsonGrowOffer4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_Gym_EventScript_WattsonRematchDecline
	cleartrainerflag TRAINER_WATTSON_4
	trainerbattle_double TRAINER_WATTSON_4, MauvilleCity_Gym_Text_WattsonGrowGo, MauvilleCity_Gym_Text_WattsonGrowDefeat4, MauvilleCity_Gym_Text_WattsonGrowNeedTwo
	msgbox MauvilleCity_Gym_Text_WattsonGrowAfter4, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_EventScript_WattsonRematchDecline::
	goto_if_eq VAR_NEW_MAUVILLE_STATE, 2, MauvilleCity_Gym_EventScript_CompletedNewMauville
	msgbox MauvilleCity_Gym_Text_WattsonPostBattle, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Gym_Text_WattsonGrowGo:
	.string "Here we go!$"

MauvilleCity_Gym_Text_WattsonGrowNeedTwo:
	.string "You need two POKéMON for this\n"
	.string "battle. Come back with more.$"

MauvilleCity_Gym_Text_WattsonGrowOffer1:
	.string "WATTSON: Wahahaha! {STR_VAR_1} BADGES!\p"
	.string "I've charged my team up to\n"
	.string "{STR_VAR_2}'s level. Want a jolt?$"

MauvilleCity_Gym_Text_WattsonGrowDefeat1:
	.string "Wahahaha! Shocked again!$"

MauvilleCity_Gym_Text_WattsonGrowAfter1:
	.string "Win your next BADGE and come back.\n"
	.string "I'll recharge in the meantime!$"

MauvilleCity_Gym_Text_WattsonGrowOffer2:
	.string "WATTSON: Eight BADGES! Wahahaha!\p"
	.string "My team runs near ELITE FOUR\n"
	.string "voltage now. One more spark?$"

MauvilleCity_Gym_Text_WattsonGrowDefeat2:
	.string "Wahahaha! You've got the current\n"
	.string "to take the LEAGUE!$"

MauvilleCity_Gym_Text_WattsonGrowAfter2:
	.string "Off to EVER GRANDE with you!$"

MauvilleCity_Gym_Text_WattsonGrowOffer3:
	.string "WATTSON: Wahahaha! The CHAMPION\n"
	.string "came to see this old man!\p"
	.string "My full team is charged.\n"
	.string "Shall we?$"

MauvilleCity_Gym_Text_WattsonGrowDefeat3:
	.string "Wahahaha! What a battle!$"

MauvilleCity_Gym_Text_WattsonGrowAfter3:
	.string "Come back anytime. This old man\n"
	.string "needs the exercise.$"

MauvilleCity_Gym_Text_WattsonGrowOffer4:
	.string "WATTSON: You defended your title!\p"
	.string "My team is at full power now.\n"
	.string "The biggest surge I've got!$"

MauvilleCity_Gym_Text_WattsonGrowDefeat4:
	.string "Wahahaha! I'm all out of juice!$"

MauvilleCity_Gym_Text_WattsonGrowAfter4:
	.string "Thank you for keeping an old\n"
	.string "man's spark alive.$"

