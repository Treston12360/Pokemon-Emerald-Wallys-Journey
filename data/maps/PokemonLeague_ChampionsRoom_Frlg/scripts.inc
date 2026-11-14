PokemonLeague_ChampionsRoom_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PokemonLeague_ChampionsRoom_OnWarp
	map_script MAP_SCRIPT_ON_RESUME, PokemonLeague_ChampionsRoom_OnResume
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PokemonLeague_ChampionsRoom_OnFrame
	.byte 0

PokemonLeague_ChampionsRoom_OnResume::
	setvar VAR_0x8004, 4
	call PokemonLeague_EventScript_DoLightingEffect
	goto_if_defeated TRAINER_ELITE_FOUR_LANCE, PokemonLeague_ChampionsRoom_EventScript_StopSceneTriggerEnd
	goto_if_defeated TRAINER_ELITE_FOUR_LANCE_2, PokemonLeague_ChampionsRoom_EventScript_StopSceneTriggerEnd
	end

PokemonLeague_ChampionsRoom_EventScript_StopSceneTriggerEnd::
	setvar VAR_TEMP_1, 1
	end

PokemonLeague_ChampionsRoom_EventScript_CheckStopTriggerSquirtle::
	goto_if_defeated TRAINER_CHAMPION_FIRST_SQUIRTLE, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	goto_if_defeated TRAINER_CHAMPION_REMATCH_SQUIRTLE, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	return

PokemonLeague_ChampionsRoom_EventScript_CheckStopTriggerBulbasaur::
	goto_if_defeated TRAINER_CHAMPION_FIRST_BULBASAUR, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	goto_if_defeated TRAINER_CHAMPION_REMATCH_BULBASAUR, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	return

PokemonLeague_ChampionsRoom_EventScript_CheckStopTriggerCharmander::
	goto_if_defeated TRAINER_CHAMPION_FIRST_CHARMANDER, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	goto_if_defeated TRAINER_CHAMPION_REMATCH_CHARMANDER, PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	return

PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger::
	setvar VAR_TEMP_1, 1
	return

PokemonLeague_ChampionsRoom_OnWarp::
	map_script_2 VAR_TEMP_1, 0, PokemonLeague_ChampionsRoom_EventScript_TurnPlayerNorth
	.2byte 0

PokemonLeague_ChampionsRoom_EventScript_TurnPlayerNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

PokemonLeague_ChampionsRoom_OnFrame::
	map_script_2 VAR_TEMP_1, 0, PokemonLeague_ChampionsRoom_EventScript_EnterRoom
	.2byte 0

PokemonLeague_ChampionsRoom_EventScript_EnterRoom::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	setflag FLAG_TEMP_2
	applymovement LOCALID_PLAYER, PokemonLeague_ChampionsRoom_Movement_PlayerEnter
	waitmovement 0
	delay 20
	call_if_unset FLAG_SYS_GAME_CLEAR, PokemonLeague_ChampionsRoom_EventScript_Intro
	call_if_set FLAG_SYS_GAME_CLEAR, PokemonLeague_ChampionsRoom_EventScript_RematchIntro
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	savebgm MUS_VICTORY_GYM_LEADER
	call_if_unset FLAG_IS_CHAMPION, PokemonLeague_ChampionsRoom_EventScript_Battle
	call_if_set FLAG_IS_CHAMPION, PokemonLeague_ChampionsRoom_EventScript_Rematch
	setflag FLAG_TEMP_4
	clearflag FLAG_TEMP_5
	setflag FLAG_DEFEATED_CHAMP
	setflag FLAG_TEMP_4
	msgbox PokemonLeague_ChampionsRoom_Text_PostBattle
	closemessage
	@ ARIA S-LEAGUE: Lance walks the player into the Hall of Fame (FireRed's Oak/rival scene removed)
	applymovement LOCALID_KANTO_CHAMPIONS_ROOM_RIVAL, PokemonLeague_ChampionsRoom_Movement_LanceExit
	applymovement LOCALID_PLAYER, PokemonLeague_ChampionsRoom_Movement_PlayerFollowLance
	waitmovement 0
	setvar VAR_TEMP_1, 1
	warp MAP_POKEMON_LEAGUE_HALL_OF_FAME, 5, 12
	waitstate
	releaseall
	end

PokemonLeague_ChampionsRoom_EventScript_QuestLogEnd::
	call PokemonLeague_ChampionsRoom_EventScript_StopSceneTrigger
	releaseall
	end

PokemonLeague_ChampionsRoom_EventScript_Battle::
	trainerbattle_no_intro TRAINER_ELITE_FOUR_LANCE, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_Rematch::
	trainerbattle_no_intro TRAINER_ELITE_FOUR_LANCE_2, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_Intro::
	msgbox PokemonLeague_ChampionsRoom_Text_Intro
	return

PokemonLeague_ChampionsRoom_EventScript_RematchIntro::
	msgbox PokemonLeague_ChampionsRoom_Text_RematchIntro
	return

PokemonLeague_ChampionsRoom_EventScript_BattleSquirtle::
	trainerbattle_no_intro TRAINER_CHAMPION_FIRST_SQUIRTLE, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_BattleBulbasaur::
	trainerbattle_no_intro TRAINER_CHAMPION_FIRST_BULBASAUR, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_BattleCharmander::
	trainerbattle_no_intro TRAINER_CHAMPION_FIRST_CHARMANDER, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_RematchSquirtle::
	trainerbattle_no_intro TRAINER_CHAMPION_REMATCH_SQUIRTLE, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_RematchBulbasaur::
	trainerbattle_no_intro TRAINER_CHAMPION_REMATCH_BULBASAUR, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_EventScript_RematchCharmander::
	trainerbattle_no_intro TRAINER_CHAMPION_REMATCH_CHARMANDER, PokemonLeague_ChampionsRoom_Text_Defeat
	return

PokemonLeague_ChampionsRoom_Movement_PlayerEnter::
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

PokemonLeague_ChampionsRoom_Movement_PlayerExit::
	delay_16
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end

PokemonLeague_ChampionsRoom_Movement_PlayerWatchOakEnter::
	delay_8
	walk_in_place_faster_down
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

PokemonLeague_ChampionsRoom_Movement_OakEnter::
	delay_16
	delay_16
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_up
	walk_up
	walk_in_place_faster_down
	delay_16
	step_end

PokemonLeague_ChampionsRoom_Movement_OakExit::
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end

PokemonLeague_ChampionsRoom_Movement_RivalWatchOakEnter::
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

PokemonLeague_ChampionsRoom_Text_Intro:: @ ARIA S-LEAGUE draft (docs/KANTO_SCRIPT.md L.3)
	.string "So you're the one.\p"
	.string "The Champion of Hoenn, here to take\n"
	.string "Kanto's challenge. The whole League\l"
	.string "has been talking about you.\p"
	.string "I'm Lance. I've held this room since\n"
	.string "Red left it empty.\p"
	.string "Kanto's had a hard few years. Team\n"
	.string "Rocket. Now these tears.\p"
	.string "People need to believe the League\n"
	.string "can still protect them. That's what\l"
	.string "this title means to me.\p"
	.string "So I won't go easy on you because\n"
	.string "you're a guest.\p"
	.string "Come at me with everything Hoenn\n"
	.string "gave you!$"

PokemonLeague_ChampionsRoom_Text_RematchIntro:: @ ARIA S-LEAGUE draft (docs/KANTO_SCRIPT.md L.3)
	.string "So you're the one.\p"
	.string "The Champion of Hoenn, here to take\n"
	.string "Kanto's challenge. The whole League\l"
	.string "has been talking about you.\p"
	.string "I'm Lance. I've held this room since\n"
	.string "Red left it empty.\p"
	.string "Kanto's had a hard few years. Team\n"
	.string "Rocket. Now these tears.\p"
	.string "People need to believe the League\n"
	.string "can still protect them. That's what\l"
	.string "this title means to me.\p"
	.string "So I won't go easy on you because\n"
	.string "you're a guest.\p"
	.string "Come at me with everything Hoenn\n"
	.string "gave you!$"

PokemonLeague_ChampionsRoom_Text_Defeat:: @ ARIA S-LEAGUE draft (docs/KANTO_SCRIPT.md L.3)
	.string "......It's over. Ha! I can't\n"
	.string "remember the last time I lost like\l"
	.string "that.$"

PokemonLeague_ChampionsRoom_Text_Victory::
	.string "はーはッ！\n"
	.string "かった！　かった！　かった！\p"
	.string "{PLAYER}に　まける　ような\n"
	.string "おれさま　では　なーい！\p"
	.string "ま！　ポケモンの\n"
	.string "てんさい　{RIVAL}さま　あいてに\l"
	.string "ここまで　よく　がんばった！\p"
	.string "ほめて　つかわす！\n"
	.string "はーッ！　はーはッはッ！$"

PokemonLeague_ChampionsRoom_Text_PostBattle:: @ ARIA S-LEAGUE draft (docs/KANTO_SCRIPT.md L.3)
	.string "You've earned the Hall of Fame.\n"
	.string "Kanto's, this time.\p"
	.string "Come on. I'll walk you in.$"

PokemonLeague_ChampionsRoom_Text_OakPlayer::
	.string "Oak: {PLAYER}!$"

PokemonLeague_ChampionsRoom_Text_OakCongratulations::
	.string "Oak: So, you've won!\n"
	.string "Sincerely, congratulations!\p"
	.string "You're the new Pokémon League\n"
	.string "Champion!\p"
	.string "You've grown up so much since you\n"
	.string "first left with {STR_VAR_1} to work\l"
	.string "on the Pokédex.\p"
	.string "{PLAYER}, you have come of age!$"

PokemonLeague_ChampionsRoom_Text_OakImDisappointedRival::
	.string "Oak: {RIVAL}…\n"
	.string "I'm disappointed in you.\p"
	.string "I came when I heard you'd beaten\n"
	.string "the Elite Four.\p"
	.string "But, when I got here, you had\n"
	.string "already lost!\p"
	.string "{RIVAL}, do you understand why\n"
	.string "you lost?\p"
	.string "You have forgotten to treat your\n"
	.string "Pokémon with love and trust.\p"
	.string "Without them, you will never\n"
	.string "become a Champ again!$"

PokemonLeague_ChampionsRoom_Text_OakPlayerComeWithMe::
	.string "Oak: {PLAYER}.\p"
	.string "You understand that your victory\n"
	.string "was not just your own doing.\p"
	.string "The bond you share with your\n"
	.string "Pokémon is marvelous.\p"
	.string "{PLAYER}!\n"
	.string "Come with me!$"

@ ARIA S-LEAGUE: Lance (6,8) -> left, up 2, right, up 4 (FireRed Oak's exit path), then gone
PokemonLeague_ChampionsRoom_Movement_LanceExit::
	walk_left
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end

@ Player (6,9) follows two steps behind
PokemonLeague_ChampionsRoom_Movement_PlayerFollowLance::
	delay_16
	delay_16
	delay_16
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end
