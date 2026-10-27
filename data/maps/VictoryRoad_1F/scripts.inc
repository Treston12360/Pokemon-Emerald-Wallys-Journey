VictoryRoad_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VictoryRoad_1F_OnTransition
	.byte 0

VictoryRoad_1F_OnTransition:
	call_if_eq VAR_VICTORY_ROAD_1F_STATE, 1, VictoryRoad_1F_EventScript_SetEntranceWallyPos1
	call_if_eq VAR_VICTORY_ROAD_1F_STATE, 2, VictoryRoad_1F_EventScript_SetEntranceWallyPos2
	end

VictoryRoad_1F_EventScript_SetEntranceWallyPos1::
	setobjectxyperm LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, 2, 24
	setobjectmovementtype LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, MOVEMENT_TYPE_FACE_DOWN
	return

VictoryRoad_1F_EventScript_SetEntranceWallyPos2::
	setobjectxyperm LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, 3, 24
	setobjectmovementtype LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, MOVEMENT_TYPE_FACE_DOWN
	return

VictoryRoad_1F_EventScript_WallyBattleTrigger1::
	lockall
	setvar VAR_0x8008, 1
	addobject LOCALID_VICTORY_ROAD_ENTRANCE_WALLY
	applymovement LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, VictoryRoad_1F_Movement_WallyApproachPlayer1
	waitmovement 0
	goto VictoryRoad_1F_EventScript_WallyEntranceBattle
	end

VictoryRoad_1F_EventScript_WallyBattleTrigger2::
	lockall
	setvar VAR_0x8008, 2
	addobject LOCALID_VICTORY_ROAD_ENTRANCE_WALLY
	applymovement LOCALID_VICTORY_ROAD_ENTRANCE_WALLY, VictoryRoad_1F_Movement_WallyApproachPlayer2
	waitmovement 0
	goto VictoryRoad_1F_EventScript_WallyEntranceBattle
	end

VictoryRoad_1F_EventScript_WallyEntranceBattle::
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox VictoryRoad_1F_Text_WallyNotGoingToLoseAnymore, MSGBOX_DEFAULT
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_VR_1, VictoryRoad_1F_Text_WallyEntranceDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	special HealPlayerParty
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, VictoryRoad_1F_EventScript_PlayerBeatWally
	msgbox VictoryRoad_1F_Text_WallyWinsEntranceBattle, MSGBOX_DEFAULT
	goto VictoryRoad_1F_EventScript_FinishEntranceBattle

VictoryRoad_1F_EventScript_PlayerBeatWally::
	msgbox VictoryRoad_1F_Text_WallyPostEntranceBattle, MSGBOX_DEFAULT

VictoryRoad_1F_EventScript_FinishEntranceBattle::
	clearflag FLAG_WALLY_NO_WHITEOUT
	clearflag FLAG_HIDE_VICTORY_ROAD_ENTRANCE_WALLY
	copyobjectxytoperm LOCALID_VICTORY_ROAD_ENTRANCE_WALLY
	setflag FLAG_DEFEATED_WALLY_VICTORY_ROAD
	copyvar VAR_VICTORY_ROAD_1F_STATE, VAR_0x8008
	releaseall
	end

VictoryRoad_1F_Movement_WallyApproachPlayer1:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

VictoryRoad_1F_Movement_WallyApproachPlayer2:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

@ This Wally appears near the entrance once his battle is triggered and remains there until the Hall of Fame is entered
VictoryRoad_1F_EventScript_EntranceWally::
	msgbox VictoryRoad_1F_Text_WallyPostEntranceBattle, MSGBOX_NPC
	end

@ This Wally appears and remains at the exit after the Hall of Fame is entered
VictoryRoad_1F_EventScript_ExitWally::
	trainerbattle_single TRAINER_WALLY_VR_2, VictoryRoad_1F_Text_WallyIntro, VictoryRoad_1F_Text_WallyDefeat
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, VictoryRoad_1F_EventScript_RematchWally
	msgbox VictoryRoad_1F_Text_WallyPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_RematchWally::
	trainerbattle_rematch TRAINER_WALLY_VR_2, VictoryRoad_1F_Text_WallyIntro, VictoryRoad_1F_Text_WallyDefeat
	msgbox VictoryRoad_1F_Text_WallyPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_Edgar::
	trainerbattle_single TRAINER_EDGAR, VictoryRoad_1F_Text_EdgarIntro, VictoryRoad_1F_Text_EdgarDefeat
	msgbox VictoryRoad_1F_Text_EdgarPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_Albert::
	trainerbattle_single TRAINER_ALBERT, VictoryRoad_1F_Text_AlbertIntro, VictoryRoad_1F_Text_AlbertDefeat
	msgbox VictoryRoad_1F_Text_AlbertPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_Hope::
	trainerbattle_single TRAINER_HOPE, VictoryRoad_1F_Text_HopeIntro, VictoryRoad_1F_Text_HopeDefeat
	msgbox VictoryRoad_1F_Text_HopePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_Quincy::
	trainerbattle_single TRAINER_QUINCY, VictoryRoad_1F_Text_QuincyIntro, VictoryRoad_1F_Text_QuincyDefeat
	msgbox VictoryRoad_1F_Text_QuincyPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_EventScript_Katelynn::
	trainerbattle_single TRAINER_KATELYNN, VictoryRoad_1F_Text_KatelynnIntro, VictoryRoad_1F_Text_KatelynnDefeat
	msgbox VictoryRoad_1F_Text_KatelynnPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_1F_Text_WallyNotGoingToLoseAnymore:
	.string "WALLY: Oh. You're still here. I was\n"
	.string "hoping you'd have quit by now.\p"
	.string "You saw the broadcast. I crushed\n"
	.string "WALLACE in front of all of HOENN.\p"
	.string "I'm CHAMPION now. Everyone you've ever\n"
	.string "beaten is beneath me. Including your\l"
	.string "dad.\p"
	.string "Four POKéMON. Half my strength. No MEGA\n"
	.string "EVOLUTION.\p"
	.string "Honestly, even that feels like bullying.\p"
	.string "Go on. Show me how far a nobody from\n"
	.string "LITTLEROOT can get.$"

VictoryRoad_1F_Text_WallyEntranceDefeat:
	.string "WALLY: …Don't flatter yourself. I\n"
	.string "could've crushed you any time I wanted.$"

VictoryRoad_1F_Text_WallyPostEntranceBattle:
	.string "WALLY: Congratulations. You beat half\n"
	.string "of me.\p"
	.string "Your dad must be so proud. Oh, wait. I\n"
	.string "beat him too.\p"
	.string "The ELITE FOUR are going to tear you\n"
	.string "apart, and I'm going to enjoy watching.\p"
	.string "If you somehow crawl up to my room, I'll\n"
	.string "end your journey myself.$"

VictoryRoad_1F_Text_WallyIntro:
	.string "WALLY: Hey, CHAMPION.\p"
	.string "I train down here between title\n"
	.string "challenges. Nobody bothers me.\p"
	.string "Want to see what I've been working\n"
	.string "on? Don't go easy.$"

VictoryRoad_1F_Text_WallyDefeat:
	.string "Still not enough. Good.\n"
	.string "That just means I keep going.$"

VictoryRoad_1F_Text_WallyPostBattle:
	.string "WALLY: When I'm ready, I'll come up\n"
	.string "those stairs and take the title back.\p"
	.string "Don't get comfortable.$"

VictoryRoad_1F_Text_EdgarIntro:
	.string "Fourth time on this road!\p"
	.string "SIDNEY beat me last year in three\n"
	.string "turns. He grinned the whole time!$"

VictoryRoad_1F_Text_EdgarDefeat:
	.string "Fourth time, same spot.\n"
	.string "See you next year, road.$"

VictoryRoad_1F_Text_EdgarPostBattle:
	.string "I've tried this road before.\n"
	.string "Every time, I turn back.\p"
	.string "Today I reached farther.\n"
	.string "You helped me see that.\p"
	.string "Go on. I'll follow when\n"
	.string "I'm ready.$"

VictoryRoad_1F_Text_AlbertIntro:
	.string "Eight BADGES, three years.\p"
	.string "I watched WALLY's title match. If he\n"
	.string "can, I can! Not losing here!$"

VictoryRoad_1F_Text_AlbertDefeat:
	.string "Three years…\n"
	.string "gone in one battle.$"

VictoryRoad_1F_Text_AlbertPostBattle:
	.string "I told myself losing here\n"
	.string "was impossible.\p"
	.string "That made me careless.\n"
	.string "You saw what I refused to.\p"
	.string "Next time I'll prepare\n"
	.string "for more than victory.$"

VictoryRoad_1F_Text_HopeIntro:
	.string "Grandma walked this road and never\n"
	.string "finished.\p"
	.string "She saw WALLACE win his title. I'll\n"
	.string "finish for her!$"

VictoryRoad_1F_Text_HopeDefeat:
	.string "Grandma… I'm sorry.\n"
	.string "You'd have liked this kid.$"

VictoryRoad_1F_Text_HopePostBattle:
	.string "You could become CHAMPION.\p"
	.string "I watched that boy from VERDANTURF\n"
	.string "take the title on TV.\p"
	.string "He looked like you do now.\n"
	.string "Tired, and not stopping.$"

VictoryRoad_1F_Text_QuincyIntro:
	.string "Want to know what VICTORY ROAD\n"
	.string "means?\p"
	.string "WALLACE told me once. Win, and I'll\n"
	.string "tell you!$"

VictoryRoad_1F_Text_QuincyDefeat:
	.string "Okay! You've earned\n"
	.string "the answer!$"

VictoryRoad_1F_Text_QuincyPostBattle:
	.string "Getting through here is the final\n"
	.string "test for anyone aiming at the title.\p"
	.string "No one carries you. No one finds\n"
	.string "the way for you.\p"
	.string "That's why it's called VICTORY ROAD.$"

VictoryRoad_1F_Text_KatelynnIntro:
	.string "I quit my job for this.\p"
	.string "DRAKE says the LEAGUE tests your\n"
	.string "heart, not your team. Come on!$"

VictoryRoad_1F_Text_KatelynnDefeat:
	.string "A disgrace… this close\n"
	.string "to the end.$"

VictoryRoad_1F_Text_KatelynnPostBattle:
	.string "I trained for months to\n"
	.string "reach this place.\p"
	.string "Losing to you hurts.\n"
	.string "I'm not going to pretend\l"
	.string "it doesn't.\p"
	.string "But I made it here.\n"
	.string "That still means something.$"

VictoryRoad_1F_Text_WallyWinsEntranceBattle:
	.string "WALLY: Pathetic. Even half of me is too\n"
	.string "much for you.\p"
	.string "Go home to LITTLEROOT, {PLAYER}.\p"
	.string "Nobody up there is waiting for you.\n"
	.string "Honestly? Nobody ever was.$"
