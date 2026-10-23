EverGrandeCity_DrakesRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, EverGrandeCity_DrakesRoom_OnFrame
	map_script MAP_SCRIPT_ON_LOAD, EverGrandeCity_DrakesRoom_OnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, EverGrandeCity_SidneysRoom_OnWarp
	.byte 0

@ Unused, Drake uses Sidneys identical OnWarp for some reason
EverGrandeCity_DrakesRoom_OnWarp:
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_DrakesRoom_EventScript_PlayerTurnNorth
	.2byte 0

EverGrandeCity_DrakesRoom_EventScript_PlayerTurnNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

EverGrandeCity_DrakesRoom_OnFrame:
	map_script_2 VAR_ELITE_4_STATE, 3, EverGrandeCity_DrakesRoom_EventScript_WalkInCloseDoor
	.2byte 0

EverGrandeCity_DrakesRoom_EventScript_WalkInCloseDoor::
	lockall
	call PokemonLeague_EliteFour_EventScript_WalkInCloseDoor
	setvar VAR_ELITE_4_STATE, 4
	releaseall
	end

EverGrandeCity_DrakesRoom_OnLoad:
	call_if_set FLAG_DEFEATED_ELITE_4_DRAKE, EverGrandeCity_DrakesRoom_EventScript_ResetAdvanceToNextRoom
	call_if_eq VAR_ELITE_4_STATE, 4, EverGrandeCity_DrakesRoom_EventScript_CloseDoor
	end

EverGrandeCity_DrakesRoom_EventScript_ResetAdvanceToNextRoom::
	call PokemonLeague_EliteFour_EventScript_ResetAdvanceToNextRoom
	return

EverGrandeCity_DrakesRoom_EventScript_CloseDoor::
	call PokemonLeague_EliteFour_EventScript_CloseDoor
	return

EverGrandeCity_DrakesRoom_EventScript_Drake::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_ELITE_4_DRAKE, EverGrandeCity_DrakesRoom_EventScript_PostBattleSpeech
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_DrakesRoom_EventScript_DrakeRematch
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_DrakesRoom_Text_IntroSpeech, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_DRAKE, EverGrandeCity_DrakesRoom_Text_Defeat
	goto EverGrandeCity_DrakesRoom_EventScript_Defeated
	end

EverGrandeCity_DrakesRoom_EventScript_PostBattleSpeech::
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_DrakesRoom_EventScript_DrakeRematchPost
	msgbox EverGrandeCity_DrakesRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_DrakesRoom_EventScript_Defeated::
	setvar VAR_0x8004, FANCOUNTER_DEFEATED_DRAKE
	special Script_TryGainNewFanFromCounter
	setflag FLAG_DEFEATED_ELITE_4_DRAKE
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox EverGrandeCity_DrakesRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_DrakesRoom_Text_IntroSpeech:
	.string "DRAKE: I am DRAKE, the\n"
	.string "last of the ELITE FOUR.\p"
	.string "I've sailed through storms\n"
	.string "that swallowed whole ships.\p"
	.string "During the crisis, I saw\n"
	.string "TRAINERS brave the waves\l"
	.string "for people they never met.\p"
	.string "Courage alone isn't enough.\n"
	.string "You must know when to act\l"
	.string "and whom you're protecting.\p"
	.string "Show me what you've learned.$"

EverGrandeCity_DrakesRoom_Text_Defeat:
	.string "DRAKE: Ha! Well fought!\p"
	.string "You've the judgment of\n"
	.string "a seasoned captain.$"

EverGrandeCity_DrakesRoom_Text_PostBattleSpeech:
	.string "I've spent my life reading\n"
	.string "the wind and the waves.\p"
	.string "Neither ever cared what\n"
	.string "I wanted of them.\p"
	.string "A good captain listens.\n"
	.string "A good TRAINER does, too.\p"
	.string "WALLY fought hard when he\n"
	.string "came through here.\p"
	.string "But I told him what I'm\n"
	.string "telling you now:\p"
	.string "Strength without listening\n"
	.string "can take you only so far.\p"
	.string "Go on. The CHAMPION awaits.$"

@ Wally's Journey: after the first Hall of Fame the Elite Four use their rematch teams (FRLG style).
EverGrandeCity_DrakesRoom_EventScript_DrakeRematch::
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_DrakesRoom_Text_RematchIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_DRAKE_2, EverGrandeCity_DrakesRoom_Text_RematchDefeat
	setvar VAR_0x8004, FANCOUNTER_DEFEATED_DRAKE
	special Script_TryGainNewFanFromCounter
	setflag FLAG_DEFEATED_ELITE_4_DRAKE
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
EverGrandeCity_DrakesRoom_EventScript_DrakeRematchPost::
	msgbox EverGrandeCity_DrakesRoom_Text_RematchPost, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_DrakesRoom_Text_RematchIntro:
	.string "DRAKE: So you came back, CHAMPION.\p"
	.string "A true captain doesn't rest on one\n"
	.string "victory. Neither do I.\p"
	.string "Show me you still have what\n"
	.string "it takes!$"

EverGrandeCity_DrakesRoom_Text_RematchDefeat:
	.string "Superb! You still have it!$"

EverGrandeCity_DrakesRoom_Text_RematchPost:
	.string "Go. Someone is waiting to take\n"
	.string "your title from you.$"

