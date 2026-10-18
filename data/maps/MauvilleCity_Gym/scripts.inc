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
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MauvilleCity_Gym_EventScript_WattsonRematch
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_SHOCK_WAVE, MauvilleCity_Gym_EventScript_GiveShockWave2
	goto_if_eq VAR_NEW_MAUVILLE_STATE, 2, MauvilleCity_Gym_EventScript_CompletedNewMauville
	msgbox MauvilleCity_Gym_Text_WattsonPostBattle, MSGBOX_DEFAULT
	release
	end

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
	setvar VAR_0x8004, TRAINER_WATTSON_1
	special WallyJourney_GetRematchTier
	goto_if_eq VAR_RESULT, 3, MauvilleCity_Gym_EventScript_WattsonRematchTier3
	goto_if_eq VAR_RESULT, 4, MauvilleCity_Gym_EventScript_WattsonRematchTier4
	goto_if_ge VAR_RESULT, 5, MauvilleCity_Gym_EventScript_WattsonRematchTier5
	trainerbattle_rematch_double TRAINER_WATTSON_1, MauvilleCity_Gym_Text_WattsonPreRematch, MauvilleCity_Gym_Text_WattsonRematchDefeat, MauvilleCity_Gym_Text_WattsonRematchNeedTwoMons
	msgbox MauvilleCity_Gym_Text_WattsonPostRematch, MSGBOX_AUTOCLOSE
	end

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
	.string "My electric soul will shatter your\n"
	.string "dreams, whoa-yeah!$"

MauvilleCity_Gym_Text_KirkDefeat:
	.string "Plugged in, amped up…\n"
	.string "and blown out!$"

MauvilleCity_Gym_Text_KirkPostBattle:
	.string "POKéMON and rock, it's all about\n"
	.string "heart, whoa-yeah!$"

MauvilleCity_Gym_Text_ShawnIntro:
	.string "I trained under WATTSON! I won't go\n"
	.string "down easy!$"

MauvilleCity_Gym_Text_ShawnDefeat:
	.string "Unplugged.\n"
	.string "Totally switched off.$"

MauvilleCity_Gym_Text_ShawnPostBattle:
	.string "WATTSON's been battling since before\n"
	.string "your daddy was born.\p"
	.string "He built half of MAUVILLE's power\n"
	.string "grid himself. The man is a legend.$"

MauvilleCity_Gym_Text_BenIntro:
	.string "This GYM's got puzzles! Fun, right?$"

MauvilleCity_Gym_Text_BenDefeat:
	.string "The doors confused ME more\n"
	.string "than you!$"

MauvilleCity_Gym_Text_BenPostBattle:
	.string "WATTSON loves his switch traps. Says\n"
	.string "a TRAINER who can't think under\l"
	.string "pressure won't last.$"

MauvilleCity_Gym_Text_VivianIntro:
	.string "My charm and my POKéMON will shock\n"
	.string "you!$"

MauvilleCity_Gym_Text_VivianDefeat:
	.string "Shocking!\n"
	.string "In the bad way!$"

MauvilleCity_Gym_Text_VivianPostBattle:
	.string "WATTSON founded this city.\p"
	.string "He wanted to roof the whole thing\n"
	.string "and run it on one generator. NEW\l"
	.string "MAUVILLE. It… didn't go well.$"

MauvilleCity_Gym_Text_AngeloIntro:
	.string "I love shiny things! Your BADGES\n"
	.string "are SO shiny!$"

MauvilleCity_Gym_Text_AngeloDefeat:
	.string "Oh… my eyes are frazzled…$"

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
