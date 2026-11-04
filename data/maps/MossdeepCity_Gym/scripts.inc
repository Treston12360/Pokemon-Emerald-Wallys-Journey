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
	.string "Yo, how's it going, Champion-bound\n"
	.string "{PLAYER}?\p"
	.string "The Gym Leaders here use\n"
	.string "Psychic-type Pokémon.\p"
	.string "If you go up against them with\n"
	.string "Fighting- or Poison-type Pokémon,\l"
	.string "they'll inflict horrible damage!\p"
	.string "Plus, they'll come at you with\n"
	.string "outstanding combination attacks.\p"
	.string "If you want to win, you'll need to show\n"
	.string "them how tight you are with your\l"
	.string "Pokémon. Go for it!$"

MossdeepCity_Gym_Text_GymGuidePostVictory:
	.string "Wow, you're astounding!\n"
	.string "You're one great Trainer!\p"
	.string "One Badge left. After that, the\n"
	.string "guards at Victory Road will check\l"
	.string "your Trainer Card. No exceptions!$"

MossdeepCity_Gym_Text_PrestonIntro:
	.string "The twins can read your mind.\n"
	.string "I can't.\p"
	.string "So I just hit really hard!$"

MossdeepCity_Gym_Text_PrestonDefeat:
	.string "I lost!\n"
	.string "My spirit needs tempering!$"

MossdeepCity_Gym_Text_PrestonPostBattle:
	.string "Tate and Liza's parents work at the\n"
	.string "Space Center.\p"
	.string "The twins grew up watching rockets\n"
	.string "from the roof. They still do.$"

MossdeepCity_Gym_Text_VirgilIntro:
	.string "I keep the twins' battle notes.\p"
	.string "They want a full report on you.\n"
	.string "Show me your talent!$"

MossdeepCity_Gym_Text_VirgilDefeat:
	.string "Spectacular!$"

MossdeepCity_Gym_Text_VirgilPostBattle:
	.string "The twins became Leaders at eleven.\n"
	.string "Youngest in Hoenn history.\p"
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
	.string "I applied for the Space Center and\n"
	.string "got turned down. Twice.\p"
	.string "Tate and Liza said a Gym needs\n"
	.string "stubborn people too.$"

MossdeepCity_Gym_Text_HannahIntro:
	.string "I tried out for the Space Center and\n"
	.string "got rejected.\p"
	.string "The twins took me in. I'm too strong\n"
	.string "for you now!$"

MossdeepCity_Gym_Text_HannahDefeat:
	.string "Okay, maybe You're the one\n"
	.string "who's too strong.$"

MossdeepCity_Gym_Text_HannahPostBattle:
	.string "I'm not going to get down about it.\p"
	.string "Liza told me once that she's scared\n"
	.string "of battling without Tate.\p"
	.string "If a Leader can be scared, I can be\n"
	.string "brave. That's my plan.$"

MossdeepCity_Gym_Text_SamanthaIntro:
	.string "I read the stars from Mossdeep's\n"
	.string "shore every night.\p"
	.string "Tonight they said a challenger would\n"
	.string "come. Here you are!$"

MossdeepCity_Gym_Text_SamanthaDefeat:
	.string "The stars didn't say I'd\n"
	.string "lose… Rude of them.$"

MossdeepCity_Gym_Text_SamanthaPostBattle:
	.string "Your future holds a fierce battle\n"
	.string "with two Leaders who think as one.\p"
	.string "The result? The stars haven't\n"
	.string "decided yet.$"

MossdeepCity_Gym_Text_MauraIntro:
	.string "I read the roads people travel.\p"
	.string "Yours started in Littleroot and\n"
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
	.string "Hmph. I predicted doom for Tate and\n"
	.string "Liza once too.\p"
	.string "They just laughed. At the same\n"
	.string "second. It was unsettling.$"

MossdeepCity_Gym_Text_NateIntro:
	.string "Hohoho. I've stared at this tile\n"
	.string "for three hours.\p"
	.string "Psychic types are patient.\n"
	.string "Are you?$"

MossdeepCity_Gym_Text_NateDefeat:
	.string "Oh my, my!\n"
	.string "You're quicker than you look.$"

MossdeepCity_Gym_Text_NatePostBattle:
	.string "I… I don't let defeat r-rattle me.\p"
	.string "The twins don't either. They lose\n"
	.string "together and get up together.$"

MossdeepCity_Gym_Text_KathleenIntro:
	.string "The Space Center sees the sky.\n"
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
	.string "Space Center.\p"
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
	.string "never make a Psychic Trainer.\p"
	.string "You don't overthink. The twins say\n"
	.string "that's what makes you scary.$"

MossdeepCity_Gym_Text_NicholasIntro:
	.string "I've trained my mind by staring at\n"
	.string "stars from the Space Center roof!\p"
	.string "Wroooar! Psychic power!$"

MossdeepCity_Gym_Text_NicholasDefeat:
	.string "Ow…\n"
	.string "I think I sprained my brain.$"

MossdeepCity_Gym_Text_NicholasPostBattle:
	.string "Next I'll develop an\n"
	.string "invincible Pokémon power! Soon!$"

MossdeepCity_Gym_Text_TateAndLizaIntro:
	.string "Tate: Hehehe… Surprised?\n"
	.string "Liza: Fufufu… Surprised?\p"
	.string "Tate: Two Leaders.\n"
	.string "Liza: One mind.\p"
	.string "Tate: Mom and Dad build rockets\n"
	.string "at the Space Center.\p"
	.string "Liza: We grew up counting the\n"
	.string "launches from the roof.\p"
	.string "Tate: Everyone said eleven was too\n"
	.string "young to lead a Gym.\p"
	.string "Liza: Then they battled us.\p"
	.string "Tate: Ready for our combination?\n"
	.string "Liza: Can you beat it?$"

MossdeepCity_Gym_Text_TateAndLizaDefeat:
	.string "Tate: What?! We saw it coming…\n"
	.string "Liza: …and still couldn't stop it!\p"
	.string "Tate: You win.\n"
	.string "Liza: This is yours.$"

MossdeepCity_Gym_Text_ReceivedMindBadge:
	.string "{PLAYER} received the Mind Badge\n"
	.string "from Tate and Liza.$"

MossdeepCity_Gym_Text_ExplainMindBadgeTakeThis:
	.string "Tate: The Mind Badge enhances the\n"
	.string "Sp. Atk and Sp. Def of Pokémon.\p"
	.string "Liza: It also lets you use the HM move\n"
	.string "Dive outside of battle.\p"
	.string "Tate: You should also take this, too.$"

MossdeepCity_Gym_Text_ExplainCalmMind:
	.string "Tate: That TM04 contains…\n"
	.string "Liza: Calm Mind!\p"
	.string "Tate: It raises Sp. Atk and…\n"
	.string "Liza: It raises Sp. Def!\p"
	.string "Tate: It's a move that's perfect…\n"
	.string "Liza: For Psychic Pokémon!\p"
	.string "… … … … … …$"

MossdeepCity_Gym_Text_RegisteredTateAndLiza:
	.string "Registered Gym Leaders Tate & Liza\n"
	.string "in the PokéNav.$"

MossdeepCity_Gym_Text_TateAndLizaPostBattle:
	.string "Tate: Looks like the bond between you\n"
	.string "and your Pokémon is far stronger than\l"
	.string "the bond that we share as twins.\p"
	.string "Liza: You will become even stronger!\n"
	.string "We've battled you, so we know.\p"
	.string "Tate: Wally was here before you.\n"
	.string "Liza: We saw his moves coming…\p"
	.string "Tate: …and he still won.\n"
	.string "Liza: We're still mad about it!$"

MossdeepCity_Gym_Text_TateAndLizaNeedTwoMons:
	.string "Tate: Hehehe… Surprised?\n"
	.string "Liza: Two Leaders!\p"
	.string "Tate: But you only have one…\n"
	.string "Liza: …Pokémon that can battle.\p"
	.string "Tate: There are two of us.\n"
	.string "Liza: So bring two Pokémon!$"

MossdeepCity_Gym_Text_GymStatue:
	.string "Mossdeep City Pokémon Gym$"

MossdeepCity_Gym_Text_GymStatueCertified:
	.string "Mossdeep City Pokémon Gym\p"
	.string "Liza And Tate's Certified Trainers:\n"
	.string "{PLAYER}$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch:
	.string "Tate: You came back!\n"
	.string "Liza: We knew you would.\p"
	.string "Tate: We've been training apart.\n"
	.string "Liza: It was awful.\p"
	.string "Tate: But now we're even stronger\n"
	.string "together.\p"
	.string "Liza: Let's show you!$"

MossdeepCity_Gym_Text_TateAndLizaRematchDefeat:
	.string "Tate: You and your Pokémon…\n"
	.string "Liza: …you're two minds, one heart.$"

MossdeepCity_Gym_Text_TateAndLizaPostRematch:
	.string "Tate: Knock us down…\n"
	.string "Liza: …and we get back up.\p"
	.string "Tate: Because we hold each other up.\n"
	.string "Liza: We always have.$"

MossdeepCity_Gym_Text_TateAndLizaRematchNeedTwoMons:
	.string "Tate: You came back!\n"
	.string "Liza: We knew you would.\p"
	.string "Tate: But you only have one…\n"
	.string "Liza: …Pokémon that can battle.\p"
	.string "Tate: Bring two.\n"
	.string "Liza: Then we'll battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch3:
	.string "Tate: We used to read your moves…\n"
	.string "Liza: …after you made them.\p"
	.string "Tate: Now we're faster.\n"
	.string "Liza: Much faster. Let's battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch4:
	.string "Tate: The Space Center asked us…\n"
	.string "Liza: …to track meteor pieces.\p"
	.string "Tate: We saw the stars up close.\n"
	.string "Liza: We felt so small.\p"
	.string "Tate: But not in battle!\n"
	.string "Liza: Never in battle!$"

MossdeepCity_Gym_Text_TateAndLizaPreRematch5:
	.string "Tate: This is it.\n"
	.string "Liza: Our best.\p"
	.string "Tate: Our minds are completely\n"
	.string "in sync.\p"
	.string "Liza: Can you keep up…\n"
	.string "Tate: …with both of us?$"

MossdeepCity_Gym_Text_TateAndLizaRematchDefeatElite:
	.string "Tate: …We lost again.\n"
	.string "Liza: …but together.$"

MossdeepCity_Gym_Text_TateAndLizaPostRematchElite:
	.string "Tate: Wally reads people really well.\n"
	.string "Liza: He reads them like a book.\p"
	.string "Tate: You don't read people…\n"
	.string "Liza: …you trust them.\p"
	.string "Tate: That's harder to beat.\n"
	.string "Liza: Much harder.$"

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
	setvar VAR_0x8004, 7
	special WallyJourney_RecordLeaderPostgameWin
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
	.string "You need two Pokémon for this\n"
	.string "battle. Come back with more.$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer1:
	.string "Tate: {STR_VAR_1} Badges...\n"
	.string "Liza: ...we saw it coming!\p"
	.string "Tate: More Badges means...\n"
	.string "Liza: ...we hold back less!\p"
	.string "Tate&Liza: Rematch?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat1:
	.string "Tate: We lost...\n"
	.string "Liza: ...together, again!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter1:
	.string "Liza: Get your last Badge!\n"
	.string "Tate: We'll be practicing!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer2:
	.string "Tate: Eight Badges!\n"
	.string "Liza: The League is next!\p"
	.string "Tate: So we'll show you...\n"
	.string "Liza: ...much more of our team!\p"
	.string "Tate&Liza: One more battle?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat2:
	.string "Tate: You're ready...\n"
	.string "Liza: ...for the League!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter2:
	.string "Liza: Good luck at Ever Grande!\n"
	.string "Tate: We'll be watching!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer3:
	.string "Tate: The Champion!\n"
	.string "Liza: Here in Mossdeep!\p"
	.string "Tate&Liza: Our full team is ready!\n"
	.string "Battle us?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat3:
	.string "Tate: Amazing...\n"
	.string "Liza: ...as expected!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter3:
	.string "Liza: We'll train harder!\n"
	.string "Tate: Together!$"

MossdeepCity_Gym_Text_TateAndLizaGrowOffer4:
	.string "Tate: You kept your title!\n"
	.string "Liza: So we brought our best!\p"
	.string "Tate&Liza: Our strongest team!\n"
	.string "Ready?$"

MossdeepCity_Gym_Text_TateAndLizaGrowDefeat4:
	.string "Tate: We couldn't...\n"
	.string "Liza: ...predict that!$"

MossdeepCity_Gym_Text_TateAndLizaGrowAfter4:
	.string "Tate&Liza: Come back anytime!$"

