EverGrandeCity_SidneysRoom_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, EverGrandeCity_SidneysRoom_OnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, EverGrandeCity_SidneysRoom_OnWarp
	map_script MAP_SCRIPT_ON_TRANSITION, EverGrandeCity_SidneysRoom_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, EverGrandeCity_SidneysRoom_OnFrame
	.byte 0

EverGrandeCity_SidneysRoom_OnTransition:
	setflag FLAG_MET_SCOTT_IN_EVERGRANDE
	setflag FLAG_HIDE_EVER_GRANDE_POKEMON_CENTER_1F_SCOTT
	end

EverGrandeCity_SidneysRoom_OnLoad:
	call_if_set FLAG_DEFEATED_ELITE_4_SIDNEY, EverGrandeCity_SidneysRoom_EventScript_ResetAdvanceToNextRoom
	call_if_eq VAR_ELITE_4_STATE, 1, EverGrandeCity_SidneysRoom_EventScript_CloseDoor
	end

EverGrandeCity_SidneysRoom_EventScript_ResetAdvanceToNextRoom::
	call PokemonLeague_EliteFour_EventScript_ResetAdvanceToNextRoom
	return

EverGrandeCity_SidneysRoom_EventScript_CloseDoor::
	call PokemonLeague_EliteFour_EventScript_CloseDoor
	return

EverGrandeCity_SidneysRoom_OnWarp:
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_SidneysRoom_EventScript_PlayerTurnNorth
	.2byte 0

EverGrandeCity_SidneysRoom_EventScript_PlayerTurnNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

EverGrandeCity_SidneysRoom_OnFrame:
	map_script_2 VAR_ELITE_4_STATE, 0, EverGrandeCity_SidneysRoom_EventScript_WalkInCloseDoor
	.2byte 0

EverGrandeCity_SidneysRoom_EventScript_WalkInCloseDoor::
	lockall
	call PokemonLeague_EliteFour_EventScript_WalkInCloseDoor
	setvar VAR_ELITE_4_STATE, 1
	releaseall
	end

EverGrandeCity_SidneysRoom_EventScript_Sidney::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_ELITE_4_SIDNEY, EverGrandeCity_SidneysRoom_EventScript_PostBattleSpeech
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_SidneysRoom_EventScript_SidneyRematch
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_SidneysRoom_Text_IntroSpeech, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_SIDNEY, EverGrandeCity_SidneysRoom_Text_Defeat
	goto EverGrandeCity_SidneysRoom_EventScript_Defeated
	end

EverGrandeCity_SidneysRoom_EventScript_PostBattleSpeech::
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_SidneysRoom_EventScript_SidneyRematchPost
	msgbox EverGrandeCity_SidneysRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_SidneysRoom_EventScript_Defeated::
	setflag FLAG_DEFEATED_ELITE_4_SIDNEY
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox EverGrandeCity_SidneysRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_SidneysRoom_Text_IntroSpeech:
	.string "SIDNEY: Hey, challenger!\n"
	.string "Glad you made it.\p"
	.string "I helped track the teams\n"
	.string "when they went underground.\p"
	.string "Found out something funny:\n"
	.string "people tell you plenty when\l"
	.string "you bother to listen.\p"
	.string "But don't expect a clue\n"
	.string "about how I battle.\p"
	.string "Come on! Let's have fun!$"

EverGrandeCity_SidneysRoom_Text_Defeat:
	.string "Hah! You got me!\p"
	.string "I tried to bait you into\n"
	.string "a mistake. You didn't bite.\p"
	.string "That's a good battle!$"

EverGrandeCity_SidneysRoom_Text_PostBattleSpeech:
	.string "SIDNEY: Know what I like\n"
	.string "about battling?\p"
	.string "Someone can surprise you\n"
	.string "even when you think you've\l"
	.string "figured them out.\p"
	.string "WALLY did that when he came\n"
	.string "through here. Now you have.\p"
	.string "Don't let anyone tell you\n"
	.string "your story's already written.$"

@ Wally's Journey: after the first Hall of Fame the Elite Four use their rematch teams (FRLG style).
EverGrandeCity_SidneysRoom_EventScript_SidneyRematch::
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_SidneysRoom_Text_RematchIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_SIDNEY_2, EverGrandeCity_SidneysRoom_Text_RematchDefeat
	setflag FLAG_DEFEATED_ELITE_4_SIDNEY
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
EverGrandeCity_SidneysRoom_EventScript_SidneyRematchPost::
	msgbox EverGrandeCity_SidneysRoom_Text_RematchPost, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_SidneysRoom_Text_RematchIntro:
	.string "SIDNEY: Well, well. The CHAMPION\n"
	.string "walked back in through the front door.\p"
	.string "You didn't think we'd sit still\n"
	.string "after you beat us, did you?\p"
	.string "This is my real team. Let's go!$"

EverGrandeCity_SidneysRoom_Text_RematchDefeat:
	.string "Hah! I still can't touch you!$"

EverGrandeCity_SidneysRoom_Text_RematchPost:
	.string "Go on. The others brought\n"
	.string "their best too.$"

