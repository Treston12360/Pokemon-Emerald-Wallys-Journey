EverGrandeCity_GlaciasRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, EverGrandeCity_GlaciasRoom_OnFrame
	map_script MAP_SCRIPT_ON_LOAD, EverGrandeCity_GlaciasRoom_OnLoad
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, EverGrandeCity_GlaciasRoom_OnWarp
	.byte 0

EverGrandeCity_GlaciasRoom_OnWarp:
	map_script_2 VAR_TEMP_1, 0, EverGrandeCity_GlaciasRoom_EventScript_PlayerTurnNorth
	.2byte 0

EverGrandeCity_GlaciasRoom_EventScript_PlayerTurnNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

EverGrandeCity_GlaciasRoom_OnFrame:
	map_script_2 VAR_ELITE_4_STATE, 2, EverGrandeCity_GlaciasRoom_EventScript_WalkInCloseDoor
	.2byte 0

EverGrandeCity_GlaciasRoom_EventScript_WalkInCloseDoor::
	lockall
	call PokemonLeague_EliteFour_EventScript_WalkInCloseDoor
	setvar VAR_ELITE_4_STATE, 3
	releaseall
	end

EverGrandeCity_GlaciasRoom_OnLoad:
	call_if_set FLAG_DEFEATED_ELITE_4_GLACIA, EverGrandeCity_GlaciasRoom_EventScript_ResetAdvanceToNextRoom
	call_if_eq VAR_ELITE_4_STATE, 3, EverGrandeCity_GlaciasRoom_EventScript_CloseDoor
	end

EverGrandeCity_GlaciasRoom_EventScript_ResetAdvanceToNextRoom::
	call PokemonLeague_EliteFour_EventScript_ResetAdvanceToNextRoom
	return

EverGrandeCity_GlaciasRoom_EventScript_CloseDoor::
	call PokemonLeague_EliteFour_EventScript_CloseDoor
	return

EverGrandeCity_GlaciasRoom_EventScript_Glacia::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_ELITE_4_GLACIA, EverGrandeCity_GlaciasRoom_EventScript_PostBattleSpeech
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_GlaciasRoom_EventScript_GlaciaRematch
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_GlaciasRoom_Text_IntroSpeech, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_GLACIA, EverGrandeCity_GlaciasRoom_Text_Defeat
	goto EverGrandeCity_GlaciasRoom_EventScript_Defeated
	end

EverGrandeCity_GlaciasRoom_EventScript_PostBattleSpeech::
	specialvar VAR_RESULT, WallyJourney_IsEliteFourRematchUnlocked
	goto_if_eq VAR_RESULT, TRUE, EverGrandeCity_GlaciasRoom_EventScript_GlaciaRematchPost
	msgbox EverGrandeCity_GlaciasRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_GlaciasRoom_EventScript_Defeated::
	setflag FLAG_DEFEATED_ELITE_4_GLACIA
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
	msgbox EverGrandeCity_GlaciasRoom_Text_PostBattleSpeech, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_GlaciasRoom_Text_IntroSpeech:
	.string "GLACIA: I am GLACIA.\n"
	.string "You may begin when ready.\p"
	.string "During the storms, we moved\n"
	.string "families to safer ground.\p"
	.string "Panic made every choice\n"
	.string "more dangerous.\p"
	.string "You have come this far.\n"
	.string "Let us see if your judgment\l"
	.string "is as steady as your team.$"

EverGrandeCity_GlaciasRoom_Text_Defeat:
	.string "GLACIA: Remarkable.\p"
	.string "You remained composed even\n"
	.string "when the match turned.\p"
	.string "You have earned this win.$"

EverGrandeCity_GlaciasRoom_Text_PostBattleSpeech:
	.string "I once believed discipline\n"
	.string "meant never showing doubt.\p"
	.string "During the crisis, I had\n"
	.string "to ask others for help.\p"
	.string "It was the correct choice.\p"
	.string "Strength also means knowing\n"
	.string "what you cannot do alone.\p"
	.string "Proceed. DRAKE awaits.$"

@ Wally's Journey: after the first Hall of Fame the Elite Four use their rematch teams (FRLG style).
EverGrandeCity_GlaciasRoom_EventScript_GlaciaRematch::
	playbgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
	msgbox EverGrandeCity_GlaciasRoom_Text_RematchIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_GLACIA_2, EverGrandeCity_GlaciasRoom_Text_RematchDefeat
	setflag FLAG_DEFEATED_ELITE_4_GLACIA
	call PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
EverGrandeCity_GlaciasRoom_EventScript_GlaciaRematchPost::
	msgbox EverGrandeCity_GlaciasRoom_Text_RematchPost, MSGBOX_DEFAULT
	release
	end

EverGrandeCity_GlaciasRoom_Text_RematchIntro:
	.string "GLACIA: The CHAMPION, here to take\n"
	.string "the challenge again?\p"
	.string "Last time I held back a little.\n"
	.string "Not today. Prepare to freeze.$"

EverGrandeCity_GlaciasRoom_Text_RematchDefeat:
	.string "...Even my coldest team melted.$"

EverGrandeCity_GlaciasRoom_Text_RematchPost:
	.string "DRAKE is next. He's been looking\n"
	.string "forward to this.$"

