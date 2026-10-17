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
@ 0: Wally (lv 70-75)  1: Wallace (74-79)  2: Steven (Meteor Falls team)  3+: Wally final (82-88)
WallyJourney_ChampionDefense_SetChallengerGfx::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WALLY
	goto_if_unset FLAG_SYS_GAME_CLEAR, Common_EventScript_NopReturn
	call_if_eq VAR_UNUSED_0x40FD, 1, WallyJourney_ChampionDefense_GfxWallace
	call_if_eq VAR_UNUSED_0x40FD, 2, WallyJourney_ChampionDefense_GfxSteven
	return

WallyJourney_ChampionDefense_GfxWallace::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_WALLACE
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
	switch VAR_UNUSED_0x40FD
	case 0, WallyJourney_ChampionDefense_First
	case 1, WallyJourney_ChampionDefense_Wallace
	case 2, WallyJourney_ChampionDefense_Steven
	goto_if_eq VAR_UNUSED_0x40FD, 3, WallyJourney_ChampionDefense_FinalFirst
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
	goto_if_ge VAR_UNUSED_0x40FD, 3, WallyJourney_ChampionDefense_NoCount
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
	.string "WALLY: There you are.\p"
	.string "You saw the broadcast. JUAN told me\n"
	.string "you watched it at his place.\p"
	.string "After our battle, WALLACE said HOENN\n"
	.string "needed a CHAMPION who was still\l"
	.string "climbing.\p"
	.string "So here's the thing, {PLAYER}.\p"
	.string "I know every POKéMON you've shown\n"
	.string "me. I know how you think.\p"
	.string "I've wanted this since MAUVILLE.\n"
	.string "To beat you at your best.\p"
	.string "Don't hold back. I won't.$"

EverGrandeCity_ChampionsRoom_Text_Defeat:
	.string "WALLY: …Heh.\p"
	.string "That's the only battle I've ever\n"
	.string "lost and enjoyed every second of.$"

EverGrandeCity_ChampionsRoom_Text_PostBattleSpeech:
	.string "WALLY: The first time we met, I\n"
	.string "couldn't even throw a POKé BALL\l"
	.string "without help.\p"
	.string "You, NORMAN, my family… everyone\n"
	.string "kept telling me I was stronger\l"
	.string "than I looked. I finally believe it.\p"
	.string "The title's yours now, {PLAYER}.\p"
	.string "Enjoy it while it lasts.\n"
	.string "I'm coming to take it back.$"

EverGrandeCity_ChampionsRoom_Text_MayAdvice:
	.string "MAY: {PLAYER}{KUN}!\p"
	.string "Here's some advice before you challenge\n"
	.string "the CHAMPION…$"

EverGrandeCity_ChampionsRoom_Text_MayItsAlreadyOver:
	.string "MAY: Huh?\n"
	.string "What, what, what?\p"
	.string "{PLAYER}{KUN}…\n"
	.string "Could it be that…\l"
	.string "It's already over?$"

EverGrandeCity_ChampionsRoom_Text_BrendanAdvice:
	.string "BRENDAN: {PLAYER}!\p"
	.string "How would you like some advice before\n"
	.string "you challenge the CHAMPION?…$"

EverGrandeCity_ChampionsRoom_Text_BrendanYouveWon:
	.string "BRENDAN: {PLAYER}…\p"
	.string "Are you trying to tell me…\n"
	.string "You've beaten the CHAMPION?$"

EverGrandeCity_ChampionsRoom_Text_BirchArriveRatePokedex:
	.string "PROF. BIRCH: See?\n"
	.string "What did I tell you, {RIVAL}?\p"
	.string "Didn't I tell you that you don't need\n"
	.string "to worry about {PLAYER}{KUN}?\p"
	.string "… … … … … … … …\n"
	.string "{PLAYER}{KUN}, you've finally done it.\p"
	.string "When I heard that you defeated your\n"
	.string "your own father at PETALBURG GYM,\l"
	.string "I thought perhaps you had a chance…\p"
	.string "But to think you've actually become\n"
	.string "the CHAMPION!\p"
	.string "Ah, yes!\p"
	.string "How is your POKéDEX coming?\n"
	.string "Here, let me see.$"

EverGrandeCity_ChampionsRoom_Text_BirchCongratulations:
	.string "PROF. BIRCH: Anyways,\n"
	.string "congratulations!\p"
	.string "Now, go proudly into the final room!$"

EverGrandeCity_ChampionsRoom_Text_WallaceComeWithMe:
	.string "WALLY: {PLAYER}{KUN}…\p"
	.string "No, let me rephrase that properly.\p"
	.string "The new CHAMPION!\p"
	.string "Come with me.$"

EverGrandeCity_ChampionsRoom_Text_WallaceWaitOutside:
	.string "WALLY: I'm sorry, but…\p"
	.string "From here on, only those TRAINERS who\n"
	.string "have become CHAMPIONS may enter.\p"
	.string "You'll have to wait outside with\n"
	.string "PROF. BIRCH.$"

EverGrandeCity_ChampionsRoom_Text_MayCongratulations:
	.string "MAY: Groan…\p"
	.string "… … … … … … … …\n"
	.string "… … … … … … … …\p"
	.string "I'm just joking! That's okay!\n"
	.string "That's the rule!\p"
	.string "{PLAYER}{KUN}!\n"
	.string "Honestly, congratulations!$"

EverGrandeCity_ChampionsRoom_Text_BrendanCongratulations:
	.string "BRENDAN: Whaaaat?! … … … … …\n"
	.string "… … … … … … … …\p"
	.string "It can't be helped if that's the rule.\p"
	.string "{PLAYER}, way to go!\n"
	.string "Congratulations!$"


WallyJourney_Text_ChampionWallyBack:
	.string "WALLY: …I heard your footsteps all\n"
	.string "the way up the stairs.$"
