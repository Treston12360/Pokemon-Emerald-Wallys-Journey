EverGrandeCity_ChampionsRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, EverGrandeCity_ChampionsRoom_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, EverGrandeCity_ChampionsRoom_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, EverGrandeCity_ChampionsRoom_OnFrame
	.byte 0

EverGrandeCity_ChampionsRoom_OnTransition:
	call Common_EventScript_SetupRivalGfxId
	call WallyJourney_ChampionDefense_SetChallengerGfx
	end

@ Wally's Journey: title-defense ladder. VAR_UNUSED_0x40FD counts defenses won.
@ Before the ladder: TRESTON, then CAMERON (neither counts).
@ 0: Wally (lv 70-75)  1: Wallace (74-79)  2: Steven (Meteor Falls team)  3+: Wally final (82-88)
WallyJourney_ChampionDefense_SetChallengerGfx::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WALLY
	goto_if_unset FLAG_SYS_GAME_CLEAR, Common_EventScript_NopReturn
	@ The very first title defense is the secret challenger TRESTON.
	goto_if_unset FLAG_WALLY_BEAT_TRESTON, WallyJourney_ChampionDefense_GfxTreston
	@ The second is CAMERON.
	goto_if_unset FLAG_WALLY_BEAT_CAMERON, WallyJourney_ChampionDefense_GfxCameron
	call_if_eq VAR_UNUSED_0x40FD, 1, WallyJourney_ChampionDefense_GfxWallace
	call_if_eq VAR_UNUSED_0x40FD, 2, WallyJourney_ChampionDefense_GfxSteven
	call_if_ge VAR_UNUSED_0x40FD, 4, WallyJourney_ChampionDefense_PickChallenger
	return

@ Wally's Journey: from the 5th title defense on, the challenger is drawn at random:
@ 0 WALLY  1 WALLACE  2 STEVEN  3 MAY/BRENDAN  4-11 a Gym Leader with their final team.
@ 12 TRESTON  13 MAXIE  14 ARCHIE (both reformed)  15 VITO (the Winstrates' son, team scales to you)  16 CAMERON.
WallyJourney_ChampionDefense_PickChallenger::
	@ TRESTON (the first title defense) is also number 12 in the random draw.
	random 17
	copyvar VAR_WALLY_TITLE_CHALLENGER, VAR_RESULT
	@ The Gym Leaders battle as a Double Battle. With fewer than two
	@ POKéMON able to battle, Wally takes the challenge instead.
	special HasEnoughMonsForDoubleBattle
	call_if_ne VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, WallyJourney_ChampionDefense_NoDoubles
	switch VAR_WALLY_TITLE_CHALLENGER
	case 1, WallyJourney_ChampionDefense_GfxWallace
	case 2, WallyJourney_ChampionDefense_GfxSteven
	case 3, WallyJourney_ChampionDefense_GfxRival
	case 4, WallyJourney_ChampionDefense_GfxRoxanne
	case 5, WallyJourney_ChampionDefense_GfxBrawly
	case 6, WallyJourney_ChampionDefense_GfxWattson
	case 7, WallyJourney_ChampionDefense_GfxFlannery
	case 8, WallyJourney_ChampionDefense_GfxNorman
	case 9, WallyJourney_ChampionDefense_GfxWinona
	case 10, WallyJourney_ChampionDefense_GfxLiza
	case 11, WallyJourney_ChampionDefense_GfxJuan
	case 12, WallyJourney_ChampionDefense_GfxTreston
	case 13, WallyJourney_ChampionDefense_GfxMaxie
	case 14, WallyJourney_ChampionDefense_GfxArchie
	case 15, WallyJourney_ChampionDefense_GfxVito
	case 16, WallyJourney_ChampionDefense_GfxCameron
	return

WallyJourney_ChampionDefense_NoDoubles::
	goto_if_lt VAR_WALLY_TITLE_CHALLENGER, 4, Common_EventScript_NopReturn
	goto_if_gt VAR_WALLY_TITLE_CHALLENGER, 11, Common_EventScript_NopReturn
	setvar VAR_WALLY_TITLE_CHALLENGER, 0
	return

WallyJourney_ChampionDefense_GfxMaxie::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_MAXIE
	return

WallyJourney_ChampionDefense_GfxArchie::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_ARCHIE
	return

WallyJourney_ChampionDefense_GfxVito::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_MAN_3
	return

WallyJourney_ChampionDefense_GfxCameron::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_COOLTRAINER_M
	return

WallyJourney_ChampionDefense_GfxTreston::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL
	return

WallyJourney_ChampionDefense_GfxRival::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_MAY_NORMAL
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Common_EventScript_NopReturn
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_BRENDAN_NORMAL
	return

WallyJourney_ChampionDefense_GfxWallace::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WALLACE
	return

WallyJourney_ChampionDefense_GfxRoxanne::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_ROXANNE
	return

WallyJourney_ChampionDefense_GfxBrawly::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_BRAWLY
	return

WallyJourney_ChampionDefense_GfxWattson::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WATTSON
	return

WallyJourney_ChampionDefense_GfxFlannery::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_FLANNERY
	return

WallyJourney_ChampionDefense_GfxNorman::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_NORMAN
	return

WallyJourney_ChampionDefense_GfxWinona::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WINONA
	return

WallyJourney_ChampionDefense_GfxLiza::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_LIZA
	return

WallyJourney_ChampionDefense_GfxJuan::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_JUAN
	return

WallyJourney_ChampionDefense_GfxSteven::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_STEVEN
	return

EverGrandeCity_ChampionsRoom_OnWarp:
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_ChampionsRoom_EventScript_PlayerTurnNorth
	.2byte 0

EverGrandeCity_ChampionsRoom_EventScript_PlayerTurnNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

EverGrandeCity_ChampionsRoom_OnFrame:
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_ChampionsRoom_EventScript_EnterRoom
	.2byte 0

EverGrandeCity_ChampionsRoom_EventScript_EnterRoom::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp4
	waitmovement 0
	delay 40
	applymovement LOCALID_PLAYER, EverGrandeCity_ChampionsRoom_Movement_PlayerApproachWallace
	waitmovement 0
	setvar VAR_TEMP_1, 1
	goto EverGrandeCity_ChampionsRoom_EventScript_Wally
	releaseall
	end

EverGrandeCity_ChampionsRoom_Movement_PlayerApproachWallace:
	walk_up
	walk_up
	step_end

EverGrandeCity_ChampionsRoom_EventScript_Wally::
	goto_if_set FLAG_SYS_GAME_CLEAR, WallyJourney_ChampionDefense_Wally
	@ Wally's Journey: Wally waits with his back to the door, then turns.
	delay 30
	msgbox WallyJourney_Text_ChampionWallyBack, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_ENCOUNTER_CHAMPION, FALSE
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, Common_Movement_FaceDown
	waitmovement 0
	delay 20
	msgbox EverGrandeCity_ChampionsRoom_Text_IntroSpeech, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLY_CHAMPION, EverGrandeCity_ChampionsRoom_Text_Defeat
	goto EverGrandeCity_ChampionsRoom_EventScript_Defeated
	end

WallyJourney_ChampionDefense_Wally::
	playbgm MUS_ENCOUNTER_CHAMPION, FALSE
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, Common_Movement_FaceDown
	waitmovement 0
	goto_if_unset FLAG_WALLY_BEAT_TRESTON, WallyJourney_ChampionPool_Treston
	goto_if_unset FLAG_WALLY_BEAT_CAMERON, WallyJourney_ChampionPool_Cameron
	switch VAR_UNUSED_0x40FD
	case 0, WallyJourney_ChampionDefense_First
	case 1, WallyJourney_ChampionDefense_Wallace
	case 2, WallyJourney_ChampionDefense_Steven
	goto_if_eq VAR_UNUSED_0x40FD, 3, WallyJourney_ChampionDefense_FinalFirst
	switch VAR_WALLY_TITLE_CHALLENGER
	case 1, WallyJourney_ChampionPool_Wallace
	case 2, WallyJourney_ChampionPool_Steven
	case 3, WallyJourney_ChampionPool_Rival
	case 4, WallyJourney_ChampionPool_Roxanne
	case 5, WallyJourney_ChampionPool_Brawly
	case 6, WallyJourney_ChampionPool_Wattson
	case 7, WallyJourney_ChampionPool_Flannery
	case 8, WallyJourney_ChampionPool_Norman
	case 9, WallyJourney_ChampionPool_Winona
	case 10, WallyJourney_ChampionPool_TateAndLiza
	case 11, WallyJourney_ChampionPool_Juan
	case 12, WallyJourney_ChampionPool_Treston
	case 13, WallyJourney_ChampionPool_Maxie
	case 14, WallyJourney_ChampionPool_Archie
	case 15, WallyJourney_ChampionPool_Vito
	case 16, WallyJourney_ChampionPool_Cameron
	msgbox WallyJourney_ChampionDefense_FinalIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLY_CHAMPION_FINAL, WallyJourney_ChampionDefense_FinalDefeat
	msgbox WallyJourney_ChampionDefense_FinalVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionDefense_First::
	msgbox WallyJourney_ChampionDefense_Intro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLY_CHAMPION_REMATCH, WallyJourney_ChampionDefense_DefeatedText
	msgbox WallyJourney_ChampionDefense_Victory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionDefense_Wallace::
	msgbox WallyJourney_ChampionDefense_WallaceIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLACE, WallyJourney_ChampionDefense_WallaceDefeat
	msgbox WallyJourney_ChampionDefense_WallaceVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionDefense_Steven::
	msgbox WallyJourney_ChampionDefense_StevenIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_STEVEN, WallyJourney_ChampionDefense_StevenDefeat
	@ Keep Steven's optional Meteor Falls battle available.
	cleartrainerflag TRAINER_STEVEN
	msgbox WallyJourney_ChampionDefense_StevenVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionDefense_FinalFirst::
	msgbox WallyJourney_ChampionDefense_FinalFirstIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLY_CHAMPION_FINAL, WallyJourney_ChampionDefense_FinalDefeat
	msgbox WallyJourney_ChampionDefense_FinalVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionDefense_Won::
	closemessage
	@ A title defense after the Elite Four rematch run sends the Gym Leaders to their final teams.
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	call_if_eq VAR_RESULT, TRUE, WallyJourney_ChampionDefense_SetE4RematchDone
	goto_if_ge VAR_UNUSED_0x40FD, 4, WallyJourney_ChampionDefense_NoCount
	addvar VAR_UNUSED_0x40FD, 1
WallyJourney_ChampionDefense_NoCount::
	warp MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, 9, 8
	waitstate
	releaseall
	end

WallyJourney_ChampionDefense_Intro:
	.string "WALLY: Comfortable up here?\p"
	.string "Good. I've been training at the\n"
	.string "BATTLE FRONTIER while you sat on\l"
	.string "my old throne.\p"
	.string "LEAGUE rules. Anyone who clears the\n"
	.string "ELITE FOUR gets a shot. Including\l"
	.string "me. Defend your title, CHAMPION.$"
WallyJourney_ChampionDefense_DefeatedText:
	.string "Still yours. For now.$"
WallyJourney_ChampionDefense_Victory:
	.string "WALLY: You're even better than last\n"
	.string "time. That's annoying.\p"
	.string "Word's going to get out that you\n"
	.string "won. You won't only be facing me\l"
	.string "from now on.\p"
	.string "I'll be back. Keep the seat warm.$"
WallyJourney_ChampionDefense_WallaceIntro:
	.string "WALLACE: Ah, {PLAYER}. Surprised?\p"
	.string "I lost this room to a boy I once\n"
	.string "guided. Then he lost it to you.\p"
	.string "JUAN says I've grown soft. I'd like\n"
	.string "to prove him wrong.\p"
	.string "Show me the elegance that took the\n"
	.string "crown. I won't hold back!$"
WallyJourney_ChampionDefense_WallaceDefeat:
	.string "Magnificent. Truly.$"
WallyJourney_ChampionDefense_WallaceVictory:
	.string "WALLACE: I see why WALLY speaks of\n"
	.string "you the way he does.\p"
	.string "I'll return to SOOTOPOLIS and train\n"
	.string "with my old mentor.\p"
	.string "Someone else is on his way, though.\n"
	.string "Someone who has watched you from\l"
	.string "the very start.$"
WallyJourney_ChampionDefense_StevenIntro:
	.string "STEVEN: Hello, {PLAYER}{KUN}.\p"
	.string "I told you once that I'd like to\n"
	.string "see how far you'd go.\p"
	.string "You went further than I imagined.\n"
	.string "So I'd like to stand in your way,\l"
	.string "just once, with everything I have.$"
WallyJourney_ChampionDefense_StevenDefeat:
	.string "That's the answer I was hoping for.$"
WallyJourney_ChampionDefense_StevenVictory:
	.string "STEVEN: Thank you. Truly.\p"
	.string "I'll be honest. WALLY asked me to\n"
	.string "go first. He wanted you at your\l"
	.string "sharpest.\p"
	.string "Next time, it's him. And he's been\n"
	.string "training like I've never seen.$"
WallyJourney_ChampionDefense_FinalFirstIntro:
	.string "WALLY: WALLACE. STEVEN. You beat\n"
	.string "both of them.\p"
	.string "I asked them to go first. I wanted\n"
	.string "you at your absolute best.\p"
	.string "No more warmups, {PLAYER}. This is\n"
	.string "the team I'd have used to defend\l"
	.string "the title against you.\p"
	.string "Let's find out who's really the\n"
	.string "best in HOENN.$"
WallyJourney_ChampionDefense_FinalIntro:
	.string "WALLY: Back again. So am I.\p"
	.string "Every time I lose to you, I go home\n"
	.string "and figure out exactly why.\p"
	.string "That's what makes me dangerous,\n"
	.string "{PLAYER}. Let's go!$"
WallyJourney_ChampionDefense_FinalDefeat:
	.string "…Yeah. You earned that.$"
WallyJourney_ChampionDefense_FinalVictory:
	.string "WALLY: I'm not done. I don't think\n"
	.string "I'll ever be done.\p"
	.string "That's a good thing, by the way.\n"
	.string "See you next time, CHAMPION.$"

EverGrandeCity_ChampionsRoom_EventScript_Defeated::
	playse SE_DOOR
	setmetatile 6, 1, METATILE_EliteFour_OpenDoorChampion_Frame, FALSE
	setmetatile 6, 2, METATILE_EliteFour_OpenDoorChampion_Opening, FALSE
	special DrawWholeMapView
	msgbox EverGrandeCity_ChampionsRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	closemessage
	playse SE_DOOR
	checkplayergender
	call_if_eq VAR_RESULT, MALE, EverGrandeCity_ChampionsRoom_EventScript_PlayMayMusic
	call_if_eq VAR_RESULT, FEMALE, EverGrandeCity_ChampionsRoom_EventScript_PlayBrendanMusic
	addobject LOCALID_CHAMPIONS_ROOM_RIVAL
	call EverGrandeCity_ChampionsRoom_EventScript_RivalApproachPlayer
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, EverGrandeCity_ChampionsRoom_EventScript_MayAdvice
	goto_if_eq VAR_RESULT, FEMALE, EverGrandeCity_ChampionsRoom_EventScript_BrendanAdvice
	end

EverGrandeCity_ChampionsRoom_EventScript_PlayMayMusic::
	playbgm MUS_ENCOUNTER_MAY, FALSE
	return

EverGrandeCity_ChampionsRoom_EventScript_PlayBrendanMusic::
	playbgm MUS_ENCOUNTER_BRENDAN, FALSE
	return

EverGrandeCity_ChampionsRoom_EventScript_MayAdvice::
	msgbox EverGrandeCity_ChampionsRoom_Text_MayAdvice, MSGBOX_DEFAULT
	delay 40
	playse SE_PIN
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, Common_Movement_Delay48
	waitmovement 0
	call EverGrandeCity_ChampionsRoom_EventScript_RivalLookBackAndForth
	msgbox EverGrandeCity_ChampionsRoom_Text_MayItsAlreadyOver, MSGBOX_DEFAULT
	goto EverGrandeCity_ChampionsRoom_EventScript_BirchArrivesExitForHoF
	end

EverGrandeCity_ChampionsRoom_EventScript_BrendanAdvice::
	msgbox EverGrandeCity_ChampionsRoom_Text_BrendanAdvice, MSGBOX_DEFAULT
	delay 40
	playse SE_PIN
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, Common_Movement_Delay48
	waitmovement 0
	call EverGrandeCity_ChampionsRoom_EventScript_RivalLookBackAndForth
	msgbox EverGrandeCity_ChampionsRoom_Text_BrendanYouveWon, MSGBOX_DEFAULT
	goto EverGrandeCity_ChampionsRoom_EventScript_BirchArrivesExitForHoF
	end

EverGrandeCity_ChampionsRoom_EventScript_BirchArrivesExitForHoF::
	closemessage
	addobject LOCALID_CHAMPIONS_ROOM_BIRCH
	applymovement LOCALID_CHAMPIONS_ROOM_BIRCH, EverGrandeCity_ChampionsRoom_Movement_BirchArrives
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox EverGrandeCity_ChampionsRoom_Text_BirchArriveRatePokedex, MSGBOX_DEFAULT
	call ProfBirch_EventScript_RatePokedex
	msgbox EverGrandeCity_ChampionsRoom_Text_BirchCongratulations, MSGBOX_DEFAULT
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 20
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox EverGrandeCity_ChampionsRoom_Text_WallaceComeWithMe, MSGBOX_DEFAULT
	closemessage
	delay 30
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, EverGrandeCity_ChampionsRoom_Movement_WallaceExitStart
	applymovement LOCALID_PLAYER, EverGrandeCity_ChampionsRoom_Movement_PlayerExitStart
	applymovement LOCALID_CHAMPIONS_ROOM_BIRCH, Common_Movement_WalkInPlaceFasterUp
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, EverGrandeCity_ChampionsRoom_Movement_RivalFollows
	waitmovement 0
	delay 20
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox EverGrandeCity_ChampionsRoom_Text_WallaceWaitOutside, MSGBOX_DEFAULT
	checkplayergender
	call_if_eq VAR_RESULT, MALE, EverGrandeCity_ChampionsRoom_EventScript_MayCongratulations
	call_if_eq VAR_RESULT, FEMALE, EverGrandeCity_ChampionsRoom_EventScript_BrendanCongratulations
	closemessage
	applymovement LOCALID_CHAMPIONS_ROOM_WALLACE, EverGrandeCity_ChampionsRoom_Movement_WallaceExit
	applymovement LOCALID_PLAYER, EverGrandeCity_ChampionsRoom_Movement_PlayerExit
	waitmovement 0
	setflag FLAG_HIDE_PETALBURG_GYM_GREETER
	warp MAP_EVER_GRANDE_CITY_HALL_OF_FAME, 7, 16
	waitstate
	releaseall
	end

EverGrandeCity_ChampionsRoom_EventScript_MayCongratulations::
	msgbox EverGrandeCity_ChampionsRoom_Text_MayCongratulations, MSGBOX_DEFAULT
	return

EverGrandeCity_ChampionsRoom_EventScript_BrendanCongratulations::
	msgbox EverGrandeCity_ChampionsRoom_Text_BrendanCongratulations, MSGBOX_DEFAULT
	return

EverGrandeCity_ChampionsRoom_EventScript_RivalApproachPlayer::
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, EverGrandeCity_ChampionsRoom_Movement_RivalApproachPlayer
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	return

EverGrandeCity_ChampionsRoom_EventScript_RivalLookBackAndForth::
	applymovement LOCALID_CHAMPIONS_ROOM_RIVAL, EverGrandeCity_ChampionsRoom_Movement_RivalLookBackAndForth
	waitmovement 0
	return

EverGrandeCity_ChampionsRoom_Movement_PlayerExitStart:
	walk_up
	walk_up
	step_end

EverGrandeCity_ChampionsRoom_Movement_PlayerExit:
	walk_up
	delay_16
	walk_up
	delay_8
	set_invisible
	step_end

EverGrandeCity_ChampionsRoom_Movement_RivalApproachPlayer:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_up
	walk_in_place_faster_right
	step_end

EverGrandeCity_ChampionsRoom_Movement_RivalLookBackAndForth:
	walk_in_place_faster_up
	delay_16
	walk_in_place_faster_right
	delay_16
	walk_in_place_faster_up
	delay_8
	walk_in_place_faster_right
	delay_16
	step_end

EverGrandeCity_ChampionsRoom_Movement_RivalFollows:
	walk_right
	walk_up
	step_end

EverGrandeCity_ChampionsRoom_Movement_WallaceExitStart:
	walk_up
	walk_up
	walk_in_place_faster_down
	step_end

EverGrandeCity_ChampionsRoom_Movement_WallaceExit:
	walk_up
	delay_8
	set_invisible
	step_end

EverGrandeCity_ChampionsRoom_Movement_BirchArrives:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_in_place_faster_left
	step_end

EverGrandeCity_ChampionsRoom_Text_IntroSpeech:
	.string "WALLY: So you actually made it. The\n"
	.string "ELITE FOUR must be getting soft.\p"
	.string "I beat WALLACE. I beat your dad. I beat\n"
	.string "every GYM in HOENN.\p"
	.string "And you? You've spent this whole\n"
	.string "journey one step behind me.\p"
	.string "I know your team. I know your tricks. I\n"
	.string "know exactly how you lose.\p"
	.string "This is my full strength. MEGA\n"
	.string "EVOLUTION included.\p"
	.string "Let's end this, {PLAYER}. Once and for\n"
	.string "all.$"

EverGrandeCity_ChampionsRoom_Text_Defeat:
	.string "WALLY: …No. No, no, no.\p"
	.string "I was supposed to be the strongest. I\n"
	.string "was supposed to finally matter.\p"
	.string "…Why does winning everything feel like\n"
	.string "this?$"

EverGrandeCity_ChampionsRoom_Text_PostBattleSpeech:
	.string "WALLY: …I called you a nobody.\p"
	.string "I told you no one was waiting for you. I\n"
	.string "mocked your dad.\p"
	.string "I laughed at WALLACE, the man who\n"
	.string "trained me.\p"
	.string "And none of it was true. I just needed\n"
	.string "it to be.\p"
	.string "In PETALBURG, you helped a sick kid who\n"
	.string "could barely breathe catch his first\l"
	.string "POKéMON.\p"
	.string "You never once made me feel small for\n"
	.string "needing help.\p"
	.string "And I spent this whole journey trying\n"
	.string "to make you feel small.\p"
	.string "I was so scared of being weak again\n"
	.string "that I turned into someone who looks\l"
	.string "down on everyone.\p"
	.string "That's not strength. That's just being\n"
	.string "afraid, loudly.\p"
	.string "I'm sorry, {PLAYER}. Not because I lost.\n"
	.string "Because I was wrong.\p"
	.string "I'm going to apologize to WALLACE. And\n"
	.string "to your dad.\p"
	.string "You earned this title. You earned it the\n"
	.string "right way.\p"
	.string "Someday I'll challenge you again.\p"
	.string "And next time, I'll be someone you'd\n"
	.string "actually want to battle.$"

EverGrandeCity_ChampionsRoom_Text_MayAdvice:
	.string "MAY: {PLAYER}! I heard you'd\n"
	.string "reached the POKéMON LEAGUE.\p"
	.string "I brought notes on WALLY's team.\n"
	.string "Before you face him, you should…$"

EverGrandeCity_ChampionsRoom_Text_MayItsAlreadyOver:
	.string "MAY: Wait. It's over already?\p"
	.string "You beat WALLY? {PLAYER},\n"
	.string "that's incredible!\p"
	.string "I knew you'd give him a fight.\n"
	.string "You won the whole thing!$"

EverGrandeCity_ChampionsRoom_Text_BrendanAdvice:
	.string "BRENDAN: {PLAYER}! I heard\n"
	.string "WALLY was waiting for you.\p"
	.string "I've been studying his team.\n"
	.string "There's something you should know…$"

EverGrandeCity_ChampionsRoom_Text_BrendanYouveWon:
	.string "BRENDAN: …You're telling me\n"
	.string "you already beat him?\p"
	.string "Of course you did. You've been\n"
	.string "surprising us since day one.$"

EverGrandeCity_ChampionsRoom_Text_BirchArriveRatePokedex:
	.string "PROF. BIRCH: There you are!\p"
	.string "{PLAYER}, I watched you grow\n"
	.string "from that first day on ROUTE 101.\p"
	.string "Then you faced your own father\n"
	.string "in PETALBURG and kept going.\p"
	.string "Now look at you. CHAMPION!\p"
	.string "I know this isn't the time, but\n"
	.string "may I see your POKéDEX? Your\l"
	.string "field notes matter, too.$"

EverGrandeCity_ChampionsRoom_Text_BirchCongratulations:
	.string "PROF. BIRCH: Anyways,\n"
	.string "congratulations!\p"
	.string "Now, go proudly into the final room!$"

EverGrandeCity_ChampionsRoom_Text_WallaceComeWithMe:
	.string "WALLY: {PLAYER}…\p"
	.string "No. Let me say it properly.\n"
	.string "CHAMPION {PLAYER}.\p"
	.string "I won't pretend it doesn't sting.\n"
	.string "But I am glad it's you.\p"
	.string "Come on. The HALL OF FAME awaits.$"

EverGrandeCity_ChampionsRoom_Text_WallaceWaitOutside:
	.string "WALLY: Only TRAINERS who've held\n"
	.string "the title can go past here.\p"
	.string "{RIVAL}, I'm sorry. You'll have\n"
	.string "to wait with PROF. BIRCH.\p"
	.string "{PLAYER}, let's go.$"

EverGrandeCity_ChampionsRoom_Text_MayCongratulations:
	.string "MAY: Just CHAMPIONS beyond here?\p"
	.string "Then go on. You've earned it.\p"
	.string "And when you're back, I want\n"
	.string "every detail. Congratulations!$"

EverGrandeCity_ChampionsRoom_Text_BrendanCongratulations:
	.string "BRENDAN: Rules are rules.\n"
	.string "We'll be right here.\p"
	.string "Dad's going to want the whole\n"
	.string "story. So will I.\p"
	.string "Congratulations, {PLAYER}!$"


WallyJourney_Text_ChampionWallyBack:
	.string "WALLY: …I heard your footsteps all\n"
	.string "the way up the stairs.$"

@ Wally's Journey: open title-defense challengers (5th defense on).
WallyJourney_ChampionPool_Wallace::
	msgbox WallyJourney_ChampionPool_WallaceIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_WALLACE, WallyJourney_ChampionPool_WallaceDefeat
	msgbox WallyJourney_ChampionPool_WallaceVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Steven::
	msgbox WallyJourney_ChampionPool_StevenIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_STEVEN, WallyJourney_ChampionPool_StevenDefeat
	cleartrainerflag TRAINER_STEVEN
	msgbox WallyJourney_ChampionPool_StevenVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Rival::
	msgbox WallyJourney_ChampionPool_RivalIntro, MSGBOX_DEFAULT
	goto WallyJourney_ChampionPool_RivalBattle
WallyJourney_ChampionPool_RivalAfter::
	msgbox WallyJourney_ChampionPool_RivalVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Roxanne::
	msgbox WallyJourney_ChampionPool_RoxanneIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_ROXANNE_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_ROXANNE_4, WallyJourney_ChampionPool_RoxanneDefeat
	msgbox WallyJourney_ChampionPool_RoxanneVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Brawly::
	msgbox WallyJourney_ChampionPool_BrawlyIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_BRAWLY_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_BRAWLY_4, WallyJourney_ChampionPool_BrawlyDefeat
	msgbox WallyJourney_ChampionPool_BrawlyVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Wattson::
	msgbox WallyJourney_ChampionPool_WattsonIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_WATTSON_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_WATTSON_4, WallyJourney_ChampionPool_WattsonDefeat
	msgbox WallyJourney_ChampionPool_WattsonVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Flannery::
	msgbox WallyJourney_ChampionPool_FlanneryIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_FLANNERY_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_FLANNERY_4, WallyJourney_ChampionPool_FlanneryDefeat
	msgbox WallyJourney_ChampionPool_FlanneryVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Norman::
	msgbox WallyJourney_ChampionPool_NormanIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_NORMAN_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_NORMAN_4, WallyJourney_ChampionPool_NormanDefeat
	msgbox WallyJourney_ChampionPool_NormanVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Winona::
	msgbox WallyJourney_ChampionPool_WinonaIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_WINONA_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_WINONA_4, WallyJourney_ChampionPool_WinonaDefeat
	msgbox WallyJourney_ChampionPool_WinonaVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_TateAndLiza::
	msgbox WallyJourney_ChampionPool_TateAndLizaIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_TATE_AND_LIZA_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_TATE_AND_LIZA_4, WallyJourney_ChampionPool_TateAndLizaDefeat
	msgbox WallyJourney_ChampionPool_TateAndLizaVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Juan::
	msgbox WallyJourney_ChampionPool_JuanIntro, MSGBOX_DEFAULT
	cleartrainerflag TRAINER_JUAN_4
	@ No-intro form: this runs from the room script, not from talking to an object.
	@ The trainer data marks the team as a Double Battle.
	trainerbattle_no_intro TRAINER_JUAN_4, WallyJourney_ChampionPool_JuanDefeat
	msgbox WallyJourney_ChampionPool_JuanVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_RivalBattle::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, WallyJourney_ChampionPool_Brendan
	goto_if_eq VAR_STARTER_MON, 0, WallyJourney_ChampionPool_MayTreecko
	goto_if_eq VAR_STARTER_MON, 1, WallyJourney_ChampionPool_MayTorchic
	trainerbattle_no_intro TRAINER_MAY_FINAL_MUDKIP, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter
WallyJourney_ChampionPool_MayTreecko::
	trainerbattle_no_intro TRAINER_MAY_FINAL_TREECKO, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter
WallyJourney_ChampionPool_MayTorchic::
	trainerbattle_no_intro TRAINER_MAY_FINAL_TORCHIC, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter
WallyJourney_ChampionPool_Brendan::
	goto_if_eq VAR_STARTER_MON, 0, WallyJourney_ChampionPool_BrendanTreecko
	goto_if_eq VAR_STARTER_MON, 1, WallyJourney_ChampionPool_BrendanTorchic
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_MUDKIP, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter
WallyJourney_ChampionPool_BrendanTreecko::
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_TREECKO, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter
WallyJourney_ChampionPool_BrendanTorchic::
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_TORCHIC, WallyJourney_ChampionPool_RivalDefeat
	goto WallyJourney_ChampionPool_RivalAfter

WallyJourney_ChampionPool_WallaceIntro:
	.string "WALLACE: I took my mentor's advice\n"
	.string "and trained in SOOTOPOLIS.\p"
	.string "The title isn't mine anymore, but\n"
	.string "my pride still is. Defend it!$"
WallyJourney_ChampionPool_WallaceDefeat:
	.string "Beautiful. Truly beautiful.$"
WallyJourney_ChampionPool_WallaceVictory:
	.string "WALLACE: I'll be back to try again.\n"
	.string "SOOTOPOLIS sends its regards.$"
WallyJourney_ChampionPool_StevenIntro:
	.string "STEVEN: I went back to my\n"
	.string "exploring for a while.\p"
	.string "I found a few things that made my\n"
	.string "team stronger. Let me show you.$"
WallyJourney_ChampionPool_StevenDefeat:
	.string "Remarkable. As always.$"
WallyJourney_ChampionPool_StevenVictory:
	.string "STEVEN: I'll head back to the caves.\n"
	.string "There's always more to dig up.$"
WallyJourney_ChampionPool_RivalIntro:
	.string "{RIVAL}: Surprise! I cleared the\n"
	.string "ELITE FOUR too.\p"
	.string "I've been training between\n"
	.string "fieldwork. Let's see if it paid off!$"
WallyJourney_ChampionPool_RivalDefeat:
	.string "Ugh. Still not enough.$"
WallyJourney_ChampionPool_RivalVictory:
	.string "{RIVAL}: Fine. Back to fieldwork.\n"
	.string "I'm still going to beat you someday!$"
WallyJourney_ChampionPool_RoxanneIntro:
	.string "ROXANNE: I closed the school for a\n"
	.string "week to make this trip.\p"
	.string "I've prepared for this exam more\n"
	.string "than for any other. Begin!$"
WallyJourney_ChampionPool_RoxanneDefeat:
	.string "Full marks. Again.$"
WallyJourney_ChampionPool_RoxanneVictory:
	.string "ROXANNE: I'll grade my own mistakes\n"
	.string "on the way home.$"
WallyJourney_ChampionPool_BrawlyIntro:
	.string "BRAWLY: Rode the tide all the way to\n"
	.string "EVER GRANDE!\p"
	.string "Your title's the biggest wave\n"
	.string "around. Let's go!$"
WallyJourney_ChampionPool_BrawlyDefeat:
	.string "Wiped out! Worth it!$"
WallyJourney_ChampionPool_BrawlyVictory:
	.string "BRAWLY: Back to the island.\n"
	.string "See you on the next swell!$"
WallyJourney_ChampionPool_WattsonIntro:
	.string "WATTSON: Wahahaha! This old man\n"
	.string "cleared the ELITE FOUR!\p"
	.string "Now let's see if I can charge\n"
	.string "past the CHAMPION!$"
WallyJourney_ChampionPool_WattsonDefeat:
	.string "Wahahaha! Not today!$"
WallyJourney_ChampionPool_WattsonVictory:
	.string "WATTSON: Back to MAUVILLE.\n"
	.string "That was the best fun I've had!$"
WallyJourney_ChampionPool_FlanneryIntro:
	.string "FLANNERY: I trained every day\n"
	.string "since our last battle.\p"
	.string "This time I'm taking that title.\n"
	.string "Here I come!$"
WallyJourney_ChampionPool_FlanneryDefeat:
	.string "Argh! So close!$"
WallyJourney_ChampionPool_FlanneryVictory:
	.string "FLANNERY: Grandpa's going to tease\n"
	.string "me about this one.$"
WallyJourney_ChampionPool_NormanIntro:
	.string "DAD: The ELITE FOUR let me through.\p"
	.string "I'm not here as your father today.\n"
	.string "I'm a challenger. Defend your title.$"
WallyJourney_ChampionPool_NormanDefeat:
	.string "...That's my kid.$"
WallyJourney_ChampionPool_NormanVictory:
	.string "DAD: Your mother is going to\n"
	.string "laugh at me. Worth it.$"
WallyJourney_ChampionPool_WinonaIntro:
	.string "WINONA: The wind carried me here.\p"
	.string "Let us see whose wings are\n"
	.string "stronger today, CHAMPION.$"
WallyJourney_ChampionPool_WinonaDefeat:
	.string "You still fly higher.$"
WallyJourney_ChampionPool_WinonaVictory:
	.string "WINONA: I'll return to FORTREE\n"
	.string "and keep training.$"
WallyJourney_ChampionPool_TateAndLizaIntro:
	.string "TATE: We cleared the ELITE FOUR!\n"
	.string "LIZA: Together, like always!\p"
	.string "TATE&LIZA: Now we want your title!$"
WallyJourney_ChampionPool_TateAndLizaDefeat:
	.string "TATE: Not again...\n"
	.string "LIZA: ...we'll be back!$"
WallyJourney_ChampionPool_TateAndLizaVictory:
	.string "LIZA: We'll practice more!\n"
	.string "TATE: Way more!$"
WallyJourney_ChampionPool_JuanIntro:
	.string "JUAN: I taught WALLACE, and I have\n"
	.string "watched you from afar.\p"
	.string "Allow me one last performance,\n"
	.string "for the title itself.$"
WallyJourney_ChampionPool_JuanDefeat:
	.string "Bravo. The title suits you.$"
WallyJourney_ChampionPool_JuanVictory:
	.string "JUAN: I am content.\n"
	.string "The stage remains yours.$"

@ Wally's Journey: MAXIE and ARCHIE, reformed, as title challengers.
WallyJourney_ChampionPool_Maxie::
	msgbox WallyJourney_ChampionPool_MaxieIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_MAXIE_REFORMED, WallyJourney_ChampionPool_MaxieDefeat
	msgbox WallyJourney_ChampionPool_MaxieVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_Archie::
	msgbox WallyJourney_ChampionPool_ArchieIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_ARCHIE_REFORMED, WallyJourney_ChampionPool_ArchieDefeat
	msgbox WallyJourney_ChampionPool_ArchieVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

@ Wally's Journey (owner request 2026-10-29): VITO, the Winstrates' son from Victory Road, as a
@ title-defense challenger. His team scales to the player (WallyJourney_ScaleVitoParty). The same
@ trainer as on Victory Road: if the player never beat him there, that battle stays available.
WallyJourney_ChampionPool_Vito::
	msgbox WallyJourney_ChampionPool_VitoIntro, MSGBOX_DEFAULT
	checktrainerflag TRAINER_VITO
	call_if_eq VAR_RESULT, FALSE, WallyJourney_ChampionPool_VitoRemember
	trainerbattle_no_intro TRAINER_VITO, WallyJourney_ChampionPool_VitoDefeat
	call_if_set FLAG_TEMP_1, WallyJourney_ChampionPool_VitoRestore
	msgbox WallyJourney_ChampionPool_VitoVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won

WallyJourney_ChampionPool_VitoRemember::
	setflag FLAG_TEMP_1
	return

WallyJourney_ChampionPool_VitoRestore::
	cleartrainerflag TRAINER_VITO
	return

WallyJourney_ChampionPool_VitoIntro:
	.string "VITO: CHAMPION! Remember me?\n"
	.string "VITO, of the WINSTRATES!\p"
	.string "I didn't go home. I kept training,\n"
	.string "just like Grandma VICKY taught me.\p"
	.string "Whatever you've got, my team can\n"
	.string "match it now.\p"
	.string "The whole WINSTRATE family, one\n"
	.string "more time. Let's go!$"

WallyJourney_ChampionPool_VitoDefeat:
	.string "Beaten again…\n"
	.string "Grandma's going to hear about this.$"

WallyJourney_ChampionPool_VitoVictory:
	.string "VITO: You really are the CHAMPION.\p"
	.string "I'll be back. A WINSTRATE never\n"
	.string "quits!$"

@ CAMERON is the second title defense (right after TRESTON), then joins the random draw.
WallyJourney_ChampionPool_Cameron::
	goto_if_set FLAG_WALLY_BEAT_CAMERON, WallyJourney_ChampionPool_CameronAgain
	msgbox WallyJourney_ChampionPool_CameronIntro, MSGBOX_DEFAULT
	goto WallyJourney_ChampionPool_CameronBattle
WallyJourney_ChampionPool_CameronAgain::
	msgbox WallyJourney_ChampionPool_CameronIntroAgain, MSGBOX_DEFAULT
WallyJourney_ChampionPool_CameronBattle::
	trainerbattle_no_intro TRAINER_CAMERON_CHALLENGER, WallyJourney_ChampionPool_CameronDefeat
	msgbox WallyJourney_ChampionPool_CameronVictory, MSGBOX_DEFAULT
	@ Like TRESTON, the first win over CAMERON doesn't move the title-defense ladder on.
	goto_if_set FLAG_WALLY_BEAT_CAMERON, WallyJourney_ChampionDefense_Won
	setflag FLAG_WALLY_BEAT_CAMERON
	closemessage
	goto WallyJourney_ChampionDefense_NoCount

WallyJourney_ChampionPool_CameronIntro:
	.string "CAMERON: So you're the CHAMPION.\p"
	.string "Name's CAMERON. I've been waiting a\n"
	.string "long time for this battle.\p"
	.string "Let's see what you've got!$"

WallyJourney_ChampionPool_CameronIntroAgain:
	.string "CAMERON: Back again, CHAMPION.\p"
	.string "I've been training since last time.\n"
	.string "Let's go!$"

WallyJourney_ChampionPool_CameronDefeat:
	.string "No way… That was close.$"

WallyJourney_ChampionPool_CameronVictory:
	.string "CAMERON: You're the real deal,\n"
	.string "CHAMPION.\p"
	.string "I'll be back, and UMBREON will be\n"
	.string "ready.$"

WallyJourney_ChampionPool_MaxieIntro:
	.string "MAXIE: CHAMPION. You remember me.\p"
	.string "I nearly dried out HOENN for\n"
	.string "a dream.\p"
	.string "TEAM MAGMA is no more.\n"
	.string "I ended it myself.\p"
	.string "Some of my old members help in\n"
	.string "FALLARBOR now. They test the ash-\l"
	.string "covered soil so the farms there\l"
	.string "can grow again.\p"
	.string "I came to see the strength of a\n"
	.string "TRAINER who fights for others.\l"
	.string "Show me.$"

WallyJourney_ChampionPool_MaxieDefeat:
	.string "MAXIE: …So this is the strength\n"
	.string "that stopped us.$"

WallyJourney_ChampionPool_MaxieVictory:
	.string "MAXIE: Even if GROUDON had given\n"
	.string "us more land, people would have\l"
	.string "filled it.\p"
	.string "The same troubles would have\n"
	.string "followed us.\p"
	.string "A new HOENN wasn't the answer.\n"
	.string "This one has to change, starting\l"
	.string "with people like me.\p"
	.string "That work is mine now.\n"
	.string "Thank you, CHAMPION.$"

WallyJourney_ChampionPool_ArchieIntro:
	.string "ARCHIE: Ahoy, CHAMPION! Didn't\n"
	.string "expect this old sea dog, did you?\p"
	.string "I almost drowned HOENN chasing\n"
	.string "KYOGRE. I won't ever forget that.\p"
	.string "TEAM AQUA's finished. What's left\n"
	.string "of my crew works with CAPT. STERN\l"
	.string "now, cleaning up the sea and\l"
	.string "paying back what we took.\p"
	.string "But a sailor still loves a good\n"
	.string "fight. Let's go!$"

WallyJourney_ChampionPool_ArchieDefeat:
	.string "ARCHIE: Hah! Sunk again,\n"
	.string "fair and square!$"

WallyJourney_ChampionPool_ArchieVictory:
	.string "ARCHIE: Say KYOGRE had given us\n"
	.string "more sea. Sooner or later, we'd\l"
	.string "have fouled that too.\p"
	.string "A new world doesn't fix the people\n"
	.string "living in it.\p"
	.string "So we fix this one, starting with\n"
	.string "ourselves.\p"
	.string "See you on the water.$"

WallyJourney_ChampionDefense_SetE4RematchDone::
	setflag FLAG_WALLY_E4_REMATCH_DONE
	return

WallyJourney_ChampionPool_Treston::
	goto_if_set FLAG_WALLY_BEAT_TRESTON, WallyJourney_ChampionPool_TrestonAgain
	msgbox WallyJourney_ChampionPool_TrestonIntro, MSGBOX_DEFAULT
	goto WallyJourney_ChampionPool_TrestonBattle
WallyJourney_ChampionPool_TrestonAgain::
	@ After he's been beaten once, TRESTON comes back with Darkrai.
	goto_if_set FLAG_WALLY_BEAT_TRESTON_2, WallyJourney_ChampionPool_TrestonThird
	msgbox WallyJourney_ChampionPool_TrestonIntroAgain, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_TRESTON_2, WallyJourney_ChampionPool_TrestonDefeat
	setflag FLAG_WALLY_BEAT_TRESTON_2
	msgbox WallyJourney_ChampionPool_TrestonVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won
@ After the Darkrai team is beaten, TRESTON also brings Deoxys.
WallyJourney_ChampionPool_TrestonThird::
	goto_if_set FLAG_WALLY_BEAT_TRESTON_3, WallyJourney_ChampionPool_TrestonFourth
	msgbox WallyJourney_ChampionPool_TrestonIntroThird, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_TRESTON_3, WallyJourney_ChampionPool_TrestonDefeat
	setflag FLAG_WALLY_BEAT_TRESTON_3
	msgbox WallyJourney_ChampionPool_TrestonVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won
@ After the Deoxys team is beaten, TRESTON's final team adds Giratina (every time from then on).
WallyJourney_ChampionPool_TrestonFourth::
	msgbox WallyJourney_ChampionPool_TrestonIntroFourth, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_TRESTON_4, WallyJourney_ChampionPool_TrestonDefeat
	msgbox WallyJourney_ChampionPool_TrestonVictory, MSGBOX_DEFAULT
	goto WallyJourney_ChampionDefense_Won
WallyJourney_ChampionPool_TrestonBattle::
	trainerbattle_no_intro TRAINER_TRESTON, WallyJourney_ChampionPool_TrestonDefeat
	msgbox WallyJourney_ChampionPool_TrestonVictory, MSGBOX_DEFAULT
	@ The first win over TRESTON doesn't move the title-defense ladder on: Wally is next.
	goto_if_set FLAG_WALLY_BEAT_TRESTON, WallyJourney_ChampionDefense_Won
	setflag FLAG_WALLY_BEAT_TRESTON
	closemessage
	goto WallyJourney_ChampionDefense_NoCount

WallyJourney_ChampionPool_TrestonIntro:
	.string "???: So you're the CHAMPION\n"
	.string "everyone keeps talking about.\p"
	.string "Name's TRESTON. I cleared this\n"
	.string "LEAGUE long before you got here.\p"
	.string "This is my dream team.\n"
	.string "Let's see if yours is better.$"
WallyJourney_ChampionPool_TrestonIntroAgain:
	.string "TRESTON: Back for round two.\p"
	.string "I found something new since last\n"
	.string "time. It's been giving my rivals\l"
	.string "nightmares. Let's go!$"
WallyJourney_ChampionPool_TrestonIntroThird:
	.string "TRESTON: You beat Darkrai. Fine.\p"
	.string "Then I'll bring something that\n"
	.string "came down from space.\l"
	.string "No more holding back!$"
WallyJourney_ChampionPool_TrestonIntroFourth:
	.string "TRESTON: Darkrai. Deoxys.\n"
	.string "You've beaten them both.\p"
	.string "So this time, the one from the\n"
	.string "other side came with me.\p"
	.string "EMBOAR and SCEPTILE are still\n"
	.string "here. They always will be. Go!$"
WallyJourney_ChampionPool_TrestonDefeat:
	.string "...Not bad. Not bad at all.$"
WallyJourney_ChampionPool_TrestonVictory:
	.string "TRESTON: You earned that title.\p"
	.string "I'll be back. Keep your team\n"
	.string "sharp.$"
