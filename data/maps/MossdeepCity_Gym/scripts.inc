MossdeepCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MossdeepCity_Gym_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, MossdeepCity_Gym_OnLoad
	.byte 0

@ NOTE: Mossdeep Gym was redesigned between R/S and E. Leftover (and now functionally unused) scripts are commented below

MossdeepCity_Gym_OnTransition:
	setflag FLAG_TEMP_HIDE_FOLLOWER @ Hide follower since it can collide with moving statues
	end

@ All the below checks are leftover from RS. FLAG_MOSSDEEP_GYM_SWITCH_X is never set
MossdeepCity_Gym_OnLoad:
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_1, MossdeepCity_Gym_EventScript_SetSwitch1Metatiles
	goto MossdeepCity_Gym_EventScript_CheckSwitch2
	end

MossdeepCity_Gym_EventScript_CheckSwitch2::
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_2, MossdeepCity_Gym_EventScript_SetSwitch2Metatiles
	goto MossdeepCity_Gym_EventScript_CheckSwitch3
	end

MossdeepCity_Gym_EventScript_CheckSwitch3::
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_3, MossdeepCity_Gym_EventScript_SetSwitch3Metatiles
	goto MossdeepCity_Gym_EventScript_CheckSwitch4
	end

MossdeepCity_Gym_EventScript_CheckSwitch4::
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_4, MossdeepCity_Gym_EventScript_SetSwitch4Metatiles
	end

@ All the below set metatile scripts are leftover from RS and are functionally unused
MossdeepCity_Gym_EventScript_SetSwitch1Metatiles::
	setmetatile 5, 5, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 2, 7, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_CheckSwitch2
	end

MossdeepCity_Gym_EventScript_SetSwitch2Metatiles::
	setmetatile 8, 14, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 8, 10, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_CheckSwitch3
	end

MossdeepCity_Gym_EventScript_SetSwitch3Metatiles::
	setmetatile 15, 17, METATILE_RSMossdeepGym_RedArrow_Left, FALSE
	setmetatile 17, 15, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_CheckSwitch4
	end

MossdeepCity_Gym_EventScript_SetSwitch4Metatiles::
	setmetatile 1, 23, METATILE_RSMossdeepGym_RedArrow_Up, FALSE
	setmetatile 5, 24, METATILE_RSMossdeepGym_Switch_Down, TRUE
	end

MossdeepCity_Gym_EventScript_TateAndLiza::
	trainerbattle_double TRAINER_TATE_AND_LIZA_1, MossdeepCity_Gym_Text_TateAndLizaIntro, MossdeepCity_Gym_Text_TateAndLizaDefeat, MossdeepCity_Gym_Text_TateAndLizaNeedTwoMons, MossdeepCity_Gym_EventScript_TateAndLizaDefeated, NO_MUSIC
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_CALM_MIND, MossdeepCity_Gym_EventScript_GiveCalmMind2
	goto MossdeepCity_Gym_EventScript_TateAndLizaRematchOffer

MossdeepCity_Gym_EventScript_TateAndLizaDefeated::
	message MossdeepCity_Gym_Text_ReceivedMindBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox MossdeepCity_Gym_Text_ExplainMindBadgeTakeThis, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_MOSSDEEP_GYM
	setflag FLAG_BADGE07_GET
	setvar VAR_WALLY_RIVAL_PENDING, 7
	setflag FLAG_HIDE_AQUA_HIDEOUT_GRUNTS
	clearflag FLAG_HIDE_SLATEPORT_CITY_STERNS_SHIPYARD_MR_BRINEY
	clearflag FLAG_UNKNOWN_0x393
	setvar VAR_0x8008, 7
	call Common_EventScript_SetGymTrainers
	clearflag FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS
	clearflag FLAG_HIDE_MOSSDEEP_CITY_TEAM_MAGMA
	clearflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_TEAM_MAGMA
	clearflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_TEAM_MAGMA
	clearflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN
	setvar VAR_MOSSDEEP_CITY_STATE, 1
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_STEVEN
	setvar VAR_MOSSDEEP_SPACE_CENTER_STATE, 1
	call MossdeepCity_Gym_EventScript_GiveCalmMind
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox MossdeepCity_Gym_Text_RegisteredTateAndLiza, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_TATE_AND_LIZA_MATCH_CALL
	release
	end

MossdeepCity_Gym_EventScript_GiveCalmMind2::
	giveitem ITEM_TM_CALM_MIND
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox MossdeepCity_Gym_Text_ExplainCalmMind, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_CALM_MIND
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

MossdeepCity_Gym_EventScript_GiveCalmMind::
	giveitem ITEM_TM_CALM_MIND
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_BagIsFull
	msgbox MossdeepCity_Gym_Text_ExplainCalmMind, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_CALM_MIND
	call WallyJourney_EventScript_GiveGymHMs
	return

MossdeepCity_Gym_EventScript_TateAndLizaRematch::
	goto MossdeepCity_Gym_EventScript_TateAndLizaRematchOffer

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
MossdeepCity_Gym_EventScript_TateAndLizaRematchTier3::
	trainerbattle_rematch_double TRAINER_TATE_AND_LIZA_1, MossdeepCity_Gym_Text_TateAndLizaPreRematch3, MossdeepCity_Gym_Text_TateAndLizaRematchDefeat, MossdeepCity_Gym_Text_TateAndLizaRematchNeedTwoMons
	msgbox MossdeepCity_Gym_Text_TateAndLizaPostRematch, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchTier4::
	trainerbattle_rematch_double TRAINER_TATE_AND_LIZA_1, MossdeepCity_Gym_Text_TateAndLizaPreRematch4, MossdeepCity_Gym_Text_TateAndLizaRematchDefeatElite, MossdeepCity_Gym_Text_TateAndLizaRematchNeedTwoMons
	msgbox MossdeepCity_Gym_Text_TateAndLizaPostRematchElite, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchTier5::
	trainerbattle_rematch_double TRAINER_TATE_AND_LIZA_1, MossdeepCity_Gym_Text_TateAndLizaPreRematch5, MossdeepCity_Gym_Text_TateAndLizaRematchDefeatElite, MossdeepCity_Gym_Text_TateAndLizaRematchNeedTwoMons
	msgbox MossdeepCity_Gym_Text_TateAndLizaPostRematchElite, MSGBOX_AUTOCLOSE
	end

@ All the below switch scripts are leftover from RS and are functionally unused
MossdeepCity_Gym_EventScript_Switch1::
	lockall
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_1, MossdeepCity_Gym_EventScript_ClearSwitch1
	setflag FLAG_MOSSDEEP_GYM_SWITCH_1
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 5, 5, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 2, 7, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed::
	special DrawWholeMapView
	playse SE_CLICK
	releaseall
	end

MossdeepCity_Gym_EventScript_ClearSwitch1::
	clearflag FLAG_MOSSDEEP_GYM_SWITCH_1
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 5, 5, METATILE_RSMossdeepGym_RedArrow_Left, FALSE
	setmetatile 2, 7, METATILE_RSMossdeepGym_Switch_Up, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_Switch2::
	lockall
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_2, MossdeepCity_Gym_EventScript_ClearSwitch2
	setflag FLAG_MOSSDEEP_GYM_SWITCH_2
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 8, 14, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 8, 10, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_ClearSwitch2::
	clearflag FLAG_MOSSDEEP_GYM_SWITCH_2
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 8, 14, METATILE_RSMossdeepGym_RedArrow_Down, FALSE
	setmetatile 8, 10, METATILE_RSMossdeepGym_Switch_Up, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_Switch3::
	lockall
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_3, MossdeepCity_Gym_EventScript_ClearSwitch3
	setflag FLAG_MOSSDEEP_GYM_SWITCH_3
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 15, 17, METATILE_RSMossdeepGym_RedArrow_Left, FALSE
	setmetatile 17, 15, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_ClearSwitch3::
	clearflag FLAG_MOSSDEEP_GYM_SWITCH_3
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 15, 17, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 17, 15, METATILE_RSMossdeepGym_Switch_Up, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_Switch4::
	lockall
	goto_if_set FLAG_MOSSDEEP_GYM_SWITCH_4, MossdeepCity_Gym_EventScript_ClearSwitch4
	setflag FLAG_MOSSDEEP_GYM_SWITCH_4
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 1, 23, METATILE_RSMossdeepGym_RedArrow_Up, FALSE
	setmetatile 5, 24, METATILE_RSMossdeepGym_Switch_Down, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_ClearSwitch4::
	clearflag FLAG_MOSSDEEP_GYM_SWITCH_4
	applymovement LOCALID_PLAYER, MossdeepCity_Gym_Movement_WaitAfterSwitchUse
	waitmovement 0
	setmetatile 1, 23, METATILE_RSMossdeepGym_RedArrow_Right, FALSE
	setmetatile 5, 24, METATILE_RSMossdeepGym_Switch_Up, TRUE
	goto MossdeepCity_Gym_EventScript_DrawMapAfterSwitchUsed
	end

MossdeepCity_Gym_EventScript_WarpToEntrance::
	lockall
	warpmossdeepgym MAP_MOSSDEEP_CITY_GYM, 7, 30
	waitstate
	releaseall
	end

MossdeepCity_Gym_EventScript_Preston::
	trainerbattle_single TRAINER_PRESTON, MossdeepCity_Gym_Text_PrestonIntro, MossdeepCity_Gym_Text_PrestonDefeat
	msgbox MossdeepCity_Gym_Text_PrestonPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Virgil::
	trainerbattle_single TRAINER_VIRGIL, MossdeepCity_Gym_Text_VirgilIntro, MossdeepCity_Gym_Text_VirgilDefeat
	msgbox MossdeepCity_Gym_Text_VirgilPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Blake::
	trainerbattle_single TRAINER_BLAKE, MossdeepCity_Gym_Text_BlakeIntro, MossdeepCity_Gym_Text_BlakeDefeat
	msgbox MossdeepCity_Gym_Text_BlakePostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Hannah::
	trainerbattle_single TRAINER_HANNAH, MossdeepCity_Gym_Text_HannahIntro, MossdeepCity_Gym_Text_HannahDefeat
	msgbox MossdeepCity_Gym_Text_HannahPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Samantha::
	trainerbattle_single TRAINER_SAMANTHA, MossdeepCity_Gym_Text_SamanthaIntro, MossdeepCity_Gym_Text_SamanthaDefeat
	msgbox MossdeepCity_Gym_Text_SamanthaPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Maura::
	trainerbattle_single TRAINER_MAURA, MossdeepCity_Gym_Text_MauraIntro, MossdeepCity_Gym_Text_MauraDefeat
	msgbox MossdeepCity_Gym_Text_MauraPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Sylvia::
	trainerbattle_single TRAINER_SYLVIA, MossdeepCity_Gym_Text_SylviaIntro, MossdeepCity_Gym_Text_SylviaDefeat
	msgbox MossdeepCity_Gym_Text_SylviaPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Nate::
	trainerbattle_single TRAINER_NATE, MossdeepCity_Gym_Text_NateIntro, MossdeepCity_Gym_Text_NateDefeat
	msgbox MossdeepCity_Gym_Text_NatePostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Macey::
	trainerbattle_single TRAINER_MACEY, MossdeepCity_Gym_Text_MaceyIntro, MossdeepCity_Gym_Text_MaceyDefeat
	msgbox MossdeepCity_Gym_Text_MaceyPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Clifford::
	trainerbattle_single TRAINER_CLIFFORD, MossdeepCity_Gym_Text_CliffordIntro, MossdeepCity_Gym_Text_CliffordDefeat
	msgbox MossdeepCity_Gym_Text_CliffordPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Nicholas::
	trainerbattle_single TRAINER_NICHOLAS, MossdeepCity_Gym_Text_NicholasIntro, MossdeepCity_Gym_Text_NicholasDefeat
	msgbox MossdeepCity_Gym_Text_NicholasPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_Kathleen::
	trainerbattle_single TRAINER_KATHLEEN, MossdeepCity_Gym_Text_KathleenIntro, MossdeepCity_Gym_Text_KathleenDefeat
	msgbox MossdeepCity_Gym_Text_KathleenPostBattle, MSGBOX_AUTOCLOSE
	end

MossdeepCity_Gym_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_MOSSDEEP_GYM, MossdeepCity_Gym_EventScript_GymGuidePostVictory
	msgbox MossdeepCity_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_EventScript_GymGuidePostVictory::
	msgbox MossdeepCity_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

@ Leftover from RS, functionally unused
MossdeepCity_Gym_Movement_WaitAfterSwitchUse:
	delay_16
	delay_16
	step_end

MossdeepCity_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE07_GET, MossdeepCity_Gym_EventScript_GymStatueCertified
	goto MossdeepCity_Gym_EventScript_GymStatue
	end

MossdeepCity_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE07_GET, MossdeepCity_Gym_EventScript_GymStatueCertified
	goto MossdeepCity_Gym_EventScript_GymStatue
	end

MossdeepCity_Gym_EventScript_GymStatueCertified::
	msgbox MossdeepCity_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

MossdeepCity_Gym_EventScript_GymStatue::
	msgbox MossdeepCity_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

MossdeepCity_Gym_EventScript_YellowFloorSwitch::
	playse SE_SWITCH
	waitse
	playse SE_REPEL
	initrotatingtilepuzzle FALSE
	moverotatingtileobjects 0
	waitmovement 0
	turnrotatingtileobjects
	waitmovement 0
	freerotatingtilepuzzle
	end

MossdeepCity_Gym_EventScript_BlueFloorSwitch::
	playse SE_SWITCH
	waitse
	playse SE_REPEL
	initrotatingtilepuzzle FALSE
	moverotatingtileobjects 1
	waitmovement 0
	turnrotatingtileobjects
	waitmovement 0
	freerotatingtilepuzzle
	end

MossdeepCity_Gym_EventScript_GreenFloorSwitch::
	playse SE_SWITCH
	waitse
	playse SE_REPEL
	initrotatingtilepuzzle FALSE
	moverotatingtileobjects 2
	waitmovement 0
	turnrotatingtileobjects
	waitmovement 0
	freerotatingtilepuzzle
	end

MossdeepCity_Gym_EventScript_PurpleFloorSwitch::
	playse SE_SWITCH
	waitse
	playse SE_REPEL
	initrotatingtilepuzzle FALSE
	moverotatingtileobjects 3
	waitmovement 0
	turnrotatingtileobjects
	waitmovement 0
	freerotatingtilepuzzle
	end

MossdeepCity_Gym_EventScript_RedFloorSwitch::
	playse SE_SWITCH
	waitse
	playse SE_REPEL
	initrotatingtilepuzzle FALSE
	moverotatingtileobjects 4
	waitmovement 0
	turnrotatingtileobjects
	waitmovement 0
	freerotatingtilepuzzle
	end

MossdeepCity_Gym_Text_GymGuideAdvice:
	.string "Yo, how's it going, CHAMPION-bound\n"
	.string "{PLAYER}?\p"
	.string "The GYM LEADERS here use\n"
	.string "PSYCHIC-type POKéMON.\p"
	.string "If you go up against them with\n"
	.string "FIGHTING- or POISON-type POKéMON,\l"
	.string "they'll inflict horrible damage!\p"
	.string "Plus, they'll come at you with\n"
	.string "outstanding combination attacks.\p"
	.string "If you want to win, you'll need to show\n"
	.string "them how tight you are with your\l"
	.string "POKéMON. Go for it!$"

MossdeepCity_Gym_Text_GymGuidePostVictory:
	.string "Wow, you're astounding!\n"
	.string "You're one great TRAINER!\p"
	.string "One BADGE left. After that, the\n"
	.string "guards at VICTORY ROAD will check\l"
	.string "your TRAINER CARD. No exceptions!$"

MossdeepCity_Gym_Text_PrestonIntro:
	.string "The twins can read your mind.\n"
	.string "I can't.\p"
	.string "So I just hit really hard!$"

MossdeepCity_Gym_Text_PrestonDefeat:
	.string "I lost!\n"
	.string "My spirit needs tempering!$"

MossdeepCity_Gym_Text_PrestonPostBattle:
	.string "TATE and LIZA's parents work at the\n"
	.string "SPACE CENTER.\p"
	.string "The twins grew up watching rockets\n"
	.string "from the roof. They still do.$"

MossdeepCity_Gym_Text_VirgilIntro:
	.string "I keep the twins' battle notes.\p"
	.string "They want a full report on you.\n"
	.string "Show me your talent!$"

MossdeepCity_Gym_Text_VirgilDefeat:
	.string "Spectacular!$"

MossdeepCity_Gym_Text_VirgilPostBattle:
	.string "The twins became LEADERS at eleven.\n"
	.string "Youngest in HOENN history.\p"
	.string "People said it was a gimmick. Then\n"
	.string "they lost to them.$"

MossdeepCity_Gym_Text_BlakeIntro:
	.string "I'm going to read your mind like\n"
	.string "the twins do!\p"
	.string "… … … … … …\n"
	.string "…You're thinking about lunch.$"

MossdeepCity_Gym_Text_BlakeDefeat:
	.string "I read the wrong mind…\n"
	.string "Mine.$"

MossdeepCity_Gym_Text_BlakePostBattle:
	.string "I applied for the SPACE CENTER and\n"
	.string "got turned down. Twice.\p"
	.string "TATE and LIZA said a GYM needs\n"
	.string "stubborn people too.$"

MossdeepCity_Gym_Text_HannahIntro:
	.string "I tried out for the SPACE CENTER and\n"
	.string "got rejected.\p"
	.string "The twins took me in. I'm too strong\n"
	.string "for you now!$"

MossdeepCity_Gym_Text_HannahDefeat:
	.string "Okay, maybe YOU'RE the one\n"
	.string "who's too strong.$"

MossdeepCity_Gym_Text_HannahPostBattle:
	.string "I'm not going to get down about it.\p"
	.string "LIZA told me once that she's scared\n"
	.string "of battling without TATE.\p"
	.string "If a LEADER can be scared, I can be\n"
	.string "brave. That's my plan.$"

MossdeepCity_Gym_Text_SamanthaIntro:
	.string "I read the stars from MOSSDEEP's\n"
	.string "shore every night.\p"
	.string "Tonight they said a challenger would\n"
	.string "come. Here you are!$"

MossdeepCity_Gym_Text_SamanthaDefeat:
	.string "The stars didn't say I'd\n"
	.string "lose… Rude of them.$"

MossdeepCity_Gym_Text_SamanthaPostBattle:
	.string "Your future holds a fierce battle\n"
	.string "with two LEADERS who think as one.\p"
	.string "The result? The stars haven't\n"
	.string "decided yet.$"

MossdeepCity_Gym_Text_MauraIntro:
	.string "I read the roads people travel.\p"
	.string "Yours started in LITTLEROOT and\n"
	.string "hasn't stopped. Come at me!$"

MossdeepCity_Gym_Text_MauraDefeat:
	.string "Every road you walked\n"
	.string "showed in that battle.$"

MossdeepCity_Gym_Text_MauraPostBattle:
	.string "When the twins were small, they got\n"
	.string "separated at the market once.\p"
	.string "They both started crying at the\n"
	.string "same second. Across town.$"

MossdeepCity_Gym_Text_SylviaIntro:
	.string "The twins see the future.\n"
	.string "I see doom.\p"
	.string "Yours, mostly.$"

MossdeepCity_Gym_Text_SylviaDefeat:
	.string "That shouldn't have happened…\n"
	.string "The omens were so clear…$"

MossdeepCity_Gym_Text_SylviaPostBattle:
	.string "Hmph. I predicted doom for TATE and\n"
	.string "LIZA once too.\p"
	.string "They just laughed. At the same\n"
	.string "second. It was unsettling.$"

MossdeepCity_Gym_Text_NateIntro:
	.string "Hohoho. I've stared at this tile\n"
	.string "for three hours.\p"
	.string "PSYCHIC types are patient.\n"
	.string "Are you?$"

MossdeepCity_Gym_Text_NateDefeat:
	.string "Oh my, my!\n"
	.string "You're quicker than you look.$"

MossdeepCity_Gym_Text_NatePostBattle:
	.string "I… I don't let defeat r-rattle me.\p"
	.string "The twins don't either. They lose\n"
	.string "together and get up together.$"

MossdeepCity_Gym_Text_KathleenIntro:
	.string "The SPACE CENTER sees the sky.\n"
	.string "I see what's beneath it.\p"
	.string "Let me show you a horrifying time!$"

MossdeepCity_Gym_Text_KathleenDefeat:
	.string "N-no! Too bright!\n"
	.string "You're too bright!$"

MossdeepCity_Gym_Text_KathleenPostBattle:
	.string "Your energy is contagious! Go away!\p"
	.string "…The twins are like that too.\n"
	.string "It's exhausting.$"

MossdeepCity_Gym_Text_CliffordIntro:
	.string "I spent thirty years calculating\n"
	.string "rocket paths.\p"
	.string "I've calculated yours, young one.$"

MossdeepCity_Gym_Text_CliffordDefeat:
	.string "My math was flawless.\n"
	.string "You were not in it.$"

MossdeepCity_Gym_Text_CliffordPostBattle:
	.string "I taught the twins' father at the\n"
	.string "SPACE CENTER.\p"
	.string "He said his children would reach\n"
	.string "farther than any rocket. He was\l"
	.string "right.$"

MossdeepCity_Gym_Text_MaceyIntro:
	.string "I've known the twins since they\n"
	.string "were in diapers.\p"
	.string "Nobody gets to them without\n"
	.string "getting past me!$"

MossdeepCity_Gym_Text_MaceyDefeat:
	.string "How are you this strong?!\n"
	.string "They're going to tease me.$"

MossdeepCity_Gym_Text_MaceyPostBattle:
	.string "Humph! You're strong, but you'd\n"
	.string "never make a PSYCHIC TRAINER.\p"
	.string "You don't overthink. The twins say\n"
	.string "that's what makes you scary.$"

MossdeepCity_Gym_Text_NicholasIntro:
	.string "I've trained my mind by staring at\n"
	.string "stars from the SPACE CENTER roof!\p"
	.string "Wroooar! Psychic power!$"

MossdeepCity_Gym_Text_NicholasDefeat:
	.string "Ow…\n"
	.string "I think I sprained my brain.$"

MossdeepCity_Gym_Text_NicholasPostBattle:
	.string "Next I'll develop an\n"
	.string "invincible POKéMON power! Soon!$"

MossdeepCity_Gym_Text_TateAndLizaIntro:
	.string "TATE: Hehehe… Surprised?\n"
	.string "LIZA: Fufufu… Surprised?\p"
	.string "TATE: Two LEADERS.\n"
	.string "LIZA: One mind.\p"
	.string "TATE: Mom and Dad build rockets\n"
	.string "at the SPACE CENTER.\p"
	.string "LIZA: We grew up counting the\n"
	.string "launches from the roof.\p"
	.string "TATE: Everyone said eleven was too\n"
	.string "young to lead a GYM.\p"
	.string "LIZA: Then they battled us.\p"
	.string "TATE: Ready for our combination?\n"
	.string "LIZA: Can you beat it?$"

MossdeepCity_Gym_Text_TateAndLizaDefeat:
	.string "TATE: What?! We saw it coming…\n"
	.string "LIZA: …and still couldn't stop it!\p"
	.string "TATE: You win.\n"
	.string "LIZA: This is yours.$"

MossdeepCity_Gym_Text_ReceivedMindBadge:
	.string "{PLAYER} received the MIND BADGE\n"
	.string "from TATE and LIZA.$"

MossdeepCity_Gym_Text_ExplainMindBadgeTakeThis:
	.string "TATE: The MIND BADGE enhances the\n"
	.string "SP. ATK and SP. DEF of POKéMON.\p"
	.string "LIZA: It also lets you use the HM move\n"
	.string "DIVE outside of battle.\p"
	.string "TATE: You should also take this, too.$"

MossdeepCity_Gym_Text_ExplainCalmMind:
	.string "TATE: That TM04 contains…\n"
	.string "LIZA: CALM MIND!\p"
	.string "TATE: It raises SP. ATK and…\n"
	.string "LIZA: It raises SP. DEF!\p"
	.string "TATE: It's a move that's perfect…\n"
	.string "LIZA: For PSYCHIC POKéMON!\p"
	.string "… … … … … …$"

MossdeepCity_Gym_Text_RegisteredTateAndLiza:
	.string "Registered GYM LEADERS TATE & LIZA\n"
	.string "in the POKéNAV.$"

MossdeepCity_Gym_Text_TateAndLizaPostBattle:
	.string "TATE: Looks like the bond between you\n"
	.string "and your POKéMON is far stronger than\l"
	.string "the bond that we share as twins.\p"
	.string "LIZA: You will become even stronger!\n"
	.string "We've battled you, so we know.\p"
	.string "TATE: WALLY was here before you.\n"
	.string "LIZA: We saw his moves coming…\p"
	.string "TATE: …and he still won.\n"
	.string "LIZA: We're still mad about it!$"

MossdeepCity_Gym_Text_TateAndLizaNeedTwoMons:
	.string "TATE: Hehehe… Surprised?\n"
	.string "LIZA: Two LEADERS!\p"
	.string "TATE: But you only have one…\n"
	.string "LIZA: …POKéMON that can battle.\p"
	.string "TATE: There are two of us.\n"
	.string "LIZA: So bring two POKéMON!$"

MossdeepCity_Gym_Text_GymStatue:
	.string "MOSSDEEP CITY POKéMON GYM$"

MossdeepCity_Gym_Text_GymStatueCertified:
	.string "MOSSDEEP CITY POKéMON GYM\p"
	.string "LIZA AND TATE'S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch:
	.string "TATE: You came back!\n"
	.string "LIZA: We knew you would.\p"
	.string "TATE: We've been training apart.\n"
	.string "LIZA: It was awful.\p"
	.string "TATE: But now we're even stronger\n"
	.string "together.\p"
	.string "LIZA: Let's show you!$"

MossdeepCity_Gym_Text_TateAndLizaRematchDefeat:
	.string "TATE: You and your POKéMON…\n"
	.string "LIZA: …you're two minds, one heart.$"

MossdeepCity_Gym_Text_TateAndLizaPostRematch:
	.string "TATE: Knock us down…\n"
	.string "LIZA: …and we get back up.\p"
	.string "TATE: Because we hold each other up.\n"
	.string "LIZA: We always have.$"

MossdeepCity_Gym_Text_TateAndLizaRematchNeedTwoMons:
	.string "TATE: You came back!\n"
	.string "LIZA: We knew you would.\p"
	.string "TATE: But you only have one…\n"
	.string "LIZA: …POKéMON that can battle.\p"
	.string "TATE: Bring two.\n"
	.string "LIZA: Then we'll battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch3:
	.string "TATE: We used to read your moves…\n"
	.string "LIZA: …after you made them.\p"
	.string "TATE: Now we're faster.\n"
	.string "LIZA: Much faster. Let's battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch4:
	.string "TATE: The SPACE CENTER asked us…\n"
	.string "LIZA: …to track meteor pieces.\p"
	.string "TATE: We saw the stars up close.\n"
	.string "LIZA: We felt so small.\p"
	.string "TATE: But not in battle!\n"
	.string "LIZA: Never in battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch5:
	.string "TATE: This is it.\n"
	.string "LIZA: Our best.\p"
	.string "TATE: Our minds are completely\n"
	.string "in sync.\p"
	.string "LIZA: Can you keep up…\n"
	.string "TATE: …with both of us?$"

MossdeepCity_Gym_Text_TateAndLizaRematchDefeatElite:
	.string "TATE: …We lost again.\n"
	.string "LIZA: …but together.$"

MossdeepCity_Gym_Text_TateAndLizaPostRematchElite:
	.string "TATE: WALLY reads people really well.\n"
	.string "LIZA: He reads them like a book.\p"
	.string "TATE: You don't read people…\n"
	.string "LIZA: …you trust them.\p"
	.string "TATE: That's harder to beat.\n"
	.string "LIZA: Much harder.$"

@ Wally's Journey: growing Gym Leader rematches (see WallyJourney_ScaleLeaderParty).
MossdeepCity_Gym_EventScript_TateAndLizaRematchOffer::
	setvar VAR_0x8004, 7
	special WallyJourney_SetupLeaderRematch
	switch VAR_RESULT
	case 1, MossdeepCity_Gym_EventScript_TateAndLizaRematchStage1
	case 2, MossdeepCity_Gym_EventScript_TateAndLizaRematchStage2
	case 3, MossdeepCity_Gym_EventScript_TateAndLizaRematchStage3
	case 4, MossdeepCity_Gym_EventScript_TateAndLizaRematchStage4
	goto MossdeepCity_Gym_EventScript_TateAndLizaRematchStage1

MossdeepCity_Gym_EventScript_TateAndLizaRematchStage1::
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowOffer1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_Gym_EventScript_TateAndLizaRematchDecline
	cleartrainerflag TRAINER_TATE_AND_LIZA_2
	trainerbattle_double TRAINER_TATE_AND_LIZA_2, MossdeepCity_Gym_Text_TateAndLizaGrowGo, MossdeepCity_Gym_Text_TateAndLizaGrowDefeat1, MossdeepCity_Gym_Text_TateAndLizaGrowNeedTwo
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowAfter1, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchStage2::
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowOffer2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_Gym_EventScript_TateAndLizaRematchDecline
	cleartrainerflag TRAINER_TATE_AND_LIZA_2
	trainerbattle_double TRAINER_TATE_AND_LIZA_2, MossdeepCity_Gym_Text_TateAndLizaGrowGo, MossdeepCity_Gym_Text_TateAndLizaGrowDefeat2, MossdeepCity_Gym_Text_TateAndLizaGrowNeedTwo
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowAfter2, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchStage3::
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowOffer3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_Gym_EventScript_TateAndLizaRematchDecline
	cleartrainerflag TRAINER_TATE_AND_LIZA_3
	trainerbattle_double TRAINER_TATE_AND_LIZA_3, MossdeepCity_Gym_Text_TateAndLizaGrowGo, MossdeepCity_Gym_Text_TateAndLizaGrowDefeat3, MossdeepCity_Gym_Text_TateAndLizaGrowNeedTwo
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowAfter3, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchStage4::
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowOffer4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_Gym_EventScript_TateAndLizaRematchDecline
	cleartrainerflag TRAINER_TATE_AND_LIZA_4
	trainerbattle_double TRAINER_TATE_AND_LIZA_4, MossdeepCity_Gym_Text_TateAndLizaGrowGo, MossdeepCity_Gym_Text_TateAndLizaGrowDefeat4, MossdeepCity_Gym_Text_TateAndLizaGrowNeedTwo
	msgbox MossdeepCity_Gym_Text_TateAndLizaGrowAfter4, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_EventScript_TateAndLizaRematchDecline::
	msgbox MossdeepCity_Gym_Text_TateAndLizaPostBattle, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Gym_Text_TateAndLizaGrowGo:
	.string "Here we go!$"

MossdeepCity_Gym_Text_TateAndLizaGrowNeedTwo:
	.string "You need two POKéMON for this\n"
	.string "battle. Come back with more.$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer1:
	.string "TATE: {STR_VAR_1} BADGES...\n"
	.string "LIZA: ...we saw it coming!\p"
	.string "TATE: Our team grew to\n"
	.string "LIZA: ...{STR_VAR_2}'s level!\p"
	.string "TATE&LIZA: Rematch?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat1:
	.string "TATE: We lost...\n"
	.string "LIZA: ...together, again!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter1:
	.string "LIZA: Get your last BADGE!\n"
	.string "TATE: We'll be practicing!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer2:
	.string "TATE: Eight BADGES!\n"
	.string "LIZA: The LEAGUE is next!\p"
	.string "TATE: Our team is almost at\n"
	.string "LIZA: ...ELITE FOUR level!\p"
	.string "TATE&LIZA: One more battle?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat2:
	.string "TATE: You're ready...\n"
	.string "LIZA: ...for the LEAGUE!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter2:
	.string "LIZA: Good luck at EVER GRANDE!\n"
	.string "TATE: We'll be watching!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer3:
	.string "TATE: The CHAMPION!\n"
	.string "LIZA: Here in MOSSDEEP!\p"
	.string "TATE&LIZA: Our full team is ready!\n"
	.string "Battle us?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat3:
	.string "TATE: Amazing...\n"
	.string "LIZA: ...as expected!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter3:
	.string "LIZA: We'll train harder!\n"
	.string "TATE: Together!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer4:
	.string "TATE: You kept your title!\n"
	.string "LIZA: So we brought our best!\p"
	.string "TATE&LIZA: Our strongest team!\n"
	.string "Ready?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat4:
	.string "TATE: We couldn't...\n"
	.string "LIZA: ...predict that!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter4:
	.string "TATE&LIZA: Come back anytime!$"

