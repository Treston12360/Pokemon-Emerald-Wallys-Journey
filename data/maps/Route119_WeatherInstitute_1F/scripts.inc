Route119_WeatherInstitute_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route119_WeatherInstitute_1F_OnTransition
	.byte 0

Route119_WeatherInstitute_1F_OnTransition:
	call_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_WeatherInstitute_1F_EventScript_SetLittleBoyPos
	end

Route119_WeatherInstitute_1F_EventScript_SetLittleBoyPos::
	setobjectxyperm LOCALID_WEATHER_INSTITUTE_LITTLE_BOY, 0, 5
	setobjectmovementtype LOCALID_WEATHER_INSTITUTE_LITTLE_BOY, MOVEMENT_TYPE_FACE_RIGHT
	return

Route119_WeatherInstitute_1F_EventScript_LittleBoy::
	lock
	faceplayer
	special GetPlayerBigGuyGirlString
	goto_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_WeatherInstitute_1F_EventScript_LittleBoyTeamAquaHere
	msgbox Route119_WeatherInstitute_1F_Text_WowYoureStrong, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_LittleBoyTeamAquaHere::
	msgbox Route119_WeatherInstitute_1F_Text_EveryoneWentUpstairs, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_InstituteWorker1::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, Route119_WeatherInstitute_1F_EventScript_StudyingRain
	setvar VAR_0x8004, 0
	call_if_set FLAG_DEFEATED_KYOGRE, Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated
	call_if_set FLAG_DEFEATED_GROUDON, Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated
	goto_if_eq VAR_0x8004, 2, Route119_WeatherInstitute_1F_EventScript_StudyingRain  @ Both defeated
	msgbox Route119_WeatherInstitute_1F_Text_NoticingAbnormalWeather, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated::
	addvar VAR_0x8004, 1
	return

Route119_WeatherInstitute_1F_EventScript_StudyingRain::
	msgbox Route119_WeatherInstitute_1F_Text_ProfStudyingRain, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_InstituteWorker2::
	msgbox Route119_WeatherInstitute_1F_Text_WhatWereAquasUpTo, MSGBOX_NPC
	end

Route119_WeatherInstitute_1F_EventScript_Bed::
	lockall
	msgbox Route119_WeatherInstitute_1F_Text_TakeRestInBed, MSGBOX_DEFAULT
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	releaseall
	end

Route119_WeatherInstitute_1F_EventScript_Grunt1::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_1, Route119_WeatherInstitute_1F_Text_Grunt1Intro, Route119_WeatherInstitute_1F_Text_Grunt1Defeat
	msgbox Route119_WeatherInstitute_1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_1F_EventScript_Grunt4::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_4, Route119_WeatherInstitute_1F_Text_Grunt4Intro, Route119_WeatherInstitute_1F_Text_Grunt4Defeat
	msgbox Route119_WeatherInstitute_1F_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_1F_Text_Grunt1Intro:
	.string "The BOSS got interested in\n"
	.string "the research they have going here,\l"
	.string "so he sent us out.\p"
	.string "You quit meddling!$"

Route119_WeatherInstitute_1F_Text_Grunt1Defeat:
	.string "Blast it…\n"
	.string "Blasted by a kid…$"

Route119_WeatherInstitute_1F_Text_Grunt1PostBattle:
	.string "I'm just a GRUNT. I'm not supposed\n"
	.string "to ask what the weather research\l"
	.string "is for.\p"
	.string "…I ask anyway. Nobody answers.$"

Route119_WeatherInstitute_1F_Text_Grunt4Intro:
	.string "Huh?\n"
	.string "What's a kid doing here?$"

Route119_WeatherInstitute_1F_Text_Grunt4Defeat:
	.string "Huh?\n"
	.string "I lost?!$"

Route119_WeatherInstitute_1F_Text_Grunt4PostBattle:
	.string "Oh, no…\n"
	.string "I'll catch an earful for losing to a kid…\p"
	.string "I should just take a nap in the bed…$"

Route119_WeatherInstitute_1F_Text_EveryoneWentUpstairs:
	.string "They made everyone go upstairs.\n"
	.string "I hid here when they came in.\p\n"
	.string "I don't know why they need our\n"
	.string "weather records so badly.$"
Route119_WeatherInstitute_1F_Text_WowYoureStrong:
	.string "You got them to leave!\p\n"
	.string "I was scared the whole time.\n"
	.string "But I watched you stand up.\p\n"
	.string "Maybe being brave means doing\n"
	.string "something even when you're scared.$"
Route119_WeatherInstitute_1F_Text_ProfStudyingRain:
	.string "A little rain feeds the forest.\n"
	.string "Too much can ruin a harvest.\p\n"
	.string "We study where it falls, how\n"
	.string "long it lasts, and who it affects.\p\n"
	.string "The charts don't show the people\n"
	.string "whose homes are in its path.$"
Route119_WeatherInstitute_1F_Text_NoticingAbnormalWeather:
	.string "These readings are alarming.\n"
	.string "Dry air over one route, storms\n"
	.string "building over the next.\p\n"
	.string "There's no seasonal pattern\n"
	.string "that accounts for both.\p\n"
	.string "I sent our report to FORTREE.\n"
	.string "Someone has to prepare.$"
Route119_WeatherInstitute_1F_Text_WhatWereAquasUpTo:
	.string "TEAM AQUA wanted our forecasts.\n"
	.string "They said the sea was suffering.\p\n"
	.string "That's a concern we share.\n"
	.string "But they threatened our staff\n"
	.string "to get the research.\p\n"
	.string "Whatever their goal, that was\n"
	.string "no way to pursue it.$"
Route119_WeatherInstitute_1F_Text_TakeRestInBed:
	.string "There's a bed…\n"
	.string "Let's take a rest.$"

