JaggedPass_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, JaggedPass_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, JaggedPass_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, JaggedPass_OnLoad
	.byte 0

JaggedPass_OnResume:
	setstepcallback STEP_CB_ASH
	call_if_eq VAR_JAGGED_PASS_STATE, 0, JaggedPass_EventScript_CheckHasMagmaEmblem
	end

JaggedPass_EventScript_CheckHasMagmaEmblem::
	checkitem ITEM_MAGMA_EMBLEM
	goto_if_eq VAR_RESULT, TRUE, JaggedPass_EventScript_SetReadyToOpenHideout
	return

JaggedPass_EventScript_SetReadyToOpenHideout::
	setvar VAR_JAGGED_PASS_STATE, 1
	return

JaggedPass_OnTransition:
	call_if_eq VAR_JAGGED_PASS_ASH_WEATHER, 1, JaggedPass_EventScript_SetWeatherAsh
	end

JaggedPass_EventScript_SetWeatherAsh::
	setweather WEATHER_VOLCANIC_ASH
	doweather
	return

JaggedPass_OnLoad:
	goto_if_le VAR_JAGGED_PASS_STATE, 1, JaggedPass_EventScript_ConcealHideoutEntrance
	end

JaggedPass_EventScript_ConcealHideoutEntrance::
	setmetatile 16, 17, METATILE_Lavaridge_RockWall, TRUE
	setmetatile 16, 18, METATILE_Lavaridge_RockWall, TRUE
	end

JaggedPass_EventScript_OpenMagmaHideout::
	lockall
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 1  @ horizontal pan
	setvar VAR_0x8006, 8  @ num shakes
	setvar VAR_0x8007, 5  @ shake delay
	special ShakeCamera
	waitstate
	msgbox JaggedPass_Text_BoulderShakingInResponseToEmblem, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1   @ vertical pan
	setvar VAR_0x8005, 1   @ horizontal pan
	setvar VAR_0x8006, 16  @ num shakes
	setvar VAR_0x8007, 3   @ shake delay
	special ShakeCamera
	waitstate
	playse SE_EFFECTIVE
	setmetatile 16, 17, METATILE_Lavaridge_CaveEntrance_Top, TRUE
	setmetatile 16, 18, METATILE_Lavaridge_CaveEntrance_Bottom, FALSE
	special DrawWholeMapView
	delay 30
	setvar VAR_JAGGED_PASS_STATE, 2
	waitse
	releaseall
	end

JaggedPass_EventScript_MagmaHideoutGuard::
	lockall
	goto_if_set FLAG_BEAT_MAGMA_GRUNT_JAGGED_PASS, JaggedPass_EventScript_GuardDefeated
	waitse
	playse SE_PIN
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_FacePlayer
	waitmovement 0
	msgbox JaggedPass_Text_GruntIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_GRUNT_JAGGED_PASS, JaggedPass_Text_GruntDefeat
	setflag FLAG_BEAT_MAGMA_GRUNT_JAGGED_PASS
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_FaceOriginalDirection
	waitmovement 0
	releaseall
	end

JaggedPass_EventScript_GuardDefeated::
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_FacePlayer
	waitmovement 0
	msgbox JaggedPass_Text_GoWhereverYouWant, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_MAGMA_HIDEOUT_GUARD, Common_Movement_FaceOriginalDirection
	waitmovement 0
	releaseall
	end

JaggedPass_EventScript_Eric::
	trainerbattle_single TRAINER_ERIC, JaggedPass_Text_EricIntro, JaggedPass_Text_EricDefeat
	msgbox JaggedPass_Text_EricPostBattle, MSGBOX_AUTOCLOSE
	end

JaggedPass_EventScript_Diana::
	trainerbattle_single TRAINER_DIANA_1, JaggedPass_Text_DianaIntro, JaggedPass_Text_DianaDefeat, JaggedPass_EventScript_RegisterDiana
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, JaggedPass_EventScript_DianaRematch
	msgbox JaggedPass_Text_DianaPostBattle, MSGBOX_DEFAULT
	release
	end

JaggedPass_EventScript_RegisterDiana::
	special PlayerFaceTrainerAfterBattle
	msgbox JaggedPass_Text_DianaRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_DIANA_1
	release
	end

JaggedPass_EventScript_DianaRematch::
	trainerbattle_rematch TRAINER_DIANA_1, JaggedPass_Text_DianaRematchIntro, JaggedPass_Text_DianaRematchDefeat
	msgbox JaggedPass_Text_DianaPostRematch, MSGBOX_AUTOCLOSE
	end

JaggedPass_EventScript_Ethan::
	trainerbattle_single TRAINER_ETHAN_1, JaggedPass_Text_EthanIntro, JaggedPass_Text_EthanDefeat, JaggedPass_EventScript_RegisterEthan
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, JaggedPass_EventScript_EthanRematch
	msgbox JaggedPass_Text_EthanPostBattle, MSGBOX_DEFAULT
	release
	end

JaggedPass_EventScript_RegisterEthan::
	special PlayerFaceTrainerAfterBattle
	msgbox JaggedPass_Text_EthanRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_ETHAN_1
	release
	end

JaggedPass_EventScript_EthanRematch::
	trainerbattle_rematch TRAINER_ETHAN_1, JaggedPass_Text_EthanRematchIntro, JaggedPass_Text_EthanRematchDefeat
	msgbox JaggedPass_Text_EthanPostRematch, MSGBOX_AUTOCLOSE
	end

JaggedPass_EventScript_Julio::
	trainerbattle_single TRAINER_JULIO, JaggedPass_Text_JulioIntro, JaggedPass_Text_JulioDefeat
	msgbox JaggedPass_Text_JulioPostBattle, MSGBOX_AUTOCLOSE
	end

JaggedPass_EventScript_Autumn::
	trainerbattle_single TRAINER_AUTUMN, JaggedPass_Text_AutumnIntro, JaggedPass_Text_AutumnDefeat
	msgbox JaggedPass_Text_AutumnPostBattle, MSGBOX_AUTOCLOSE
	end

JaggedPass_Text_EricIntro:
	.string "JAGGED PASS! This bumpy road rocks\n"
	.string "my soul!\p"
	.string "The ash on these rocks fell before\n"
	.string "I was born. Respect it!$"

JaggedPass_Text_EricDefeat:
	.string "Losing left a bitter taste.\n"
	.string "Like the ash up here.$"

JaggedPass_Text_EricPostBattle:
	.string "I lost at POKéMON, but in love of the\n"
	.string "mountains, I win.\p"
	.string "This mountain made this valley.\n"
	.string "Nobody should mess with it.$"

JaggedPass_Text_DianaIntro:
	.string "No picnics on a volcano, they say!\p"
	.string "Grandma says a giant of the land\n"
	.string "sleeps under it. I'm being quiet!$"

JaggedPass_Text_DianaDefeat:
	.string "Ohhh! The ground's too\n"
	.string "bumpy for picnics AND battles!$"

JaggedPass_Text_DianaPostBattle:
	.string "Some people ride bikes up this bumpy\n"
	.string "pass. Show-offs.$"

JaggedPass_Text_DianaRegister:
	.string "Will you ever be back in this area?\n"
	.string "If you do return, I'd like a rematch.$"

JaggedPass_Text_DianaRematchIntro:
	.string "I brought a picnic again! This\n"
	.string "time with less ash!$"

JaggedPass_Text_DianaRematchDefeat:
	.string "It's the bumps!\n"
	.string "It's always the bumps!$"

JaggedPass_Text_DianaPostRematch:
	.string "My sandwich has ash on it again.\p"
	.string "FALLARBOR folks say you stop\n"
	.string "noticing the taste. I haven't.$"

JaggedPass_Text_EthanIntro:
	.string "Every step here could slip!\p"
	.string "These rocks were lava in the ash\n"
	.string "years. That's why I train here!$"

JaggedPass_Text_EthanDefeat:
	.string "It was over before we found\n"
	.string "our footing…$"

JaggedPass_Text_EthanPostBattle:
	.string "With an ACRO BIKE I could jump these\n"
	.string "ledges.$"

JaggedPass_Text_EthanRegister:
	.string "When I get more used to this bumpiness,\n"
	.string "I'll be sure to win!\p"
	.string "Can you register me in your POKéNAV?$"

JaggedPass_Text_EthanRematchIntro:
	.string "I've climbed this so much I sing\n"
	.string "now. Battle!$"

JaggedPass_Text_EthanRematchDefeat:
	.string "Singing didn't help my\n"
	.string "footing…$"

JaggedPass_Text_EthanPostRematch:
	.string "Maybe RYDEL's ACRO BIKE would\n"
	.string "handle these bumps.$"

JaggedPass_Text_GruntIntro:
	.string "Wah! What are you doing here?\p"
	.string "What am I doing here? None of your\n"
	.string "business!$"

JaggedPass_Text_GruntDefeat:
	.string "Urrrgh…\p"
	.string "I joined to build houses for\n"
	.string "FALLARBOR farmers.\p"
	.string "When did it turn into guarding\n"
	.string "a hole in a mountain?$"

JaggedPass_Text_GoWhereverYouWant:
	.string "Okay, you're strong!\p"
	.string "Go wherever you want. I'm not paid\n"
	.string "enough for this.$"

JaggedPass_Text_BoulderShakingInResponseToEmblem:
	.string "Oh! This boulder is shaking in response\n"
	.string "to the MAGMA EMBLEM!$"

JaggedPass_Text_JulioIntro:
	.string "Aiyeeh! Riding this trail in one go!\p"
	.string "The LAND INITIATIVE paved half of it.\n"
	.string "Then they put on red hoods. Move!$"

JaggedPass_Text_JulioDefeat:
	.string "I'm falling apart…\n"
	.string "like this trail.$"

JaggedPass_Text_JulioPostBattle:
	.string "My bike bounced so much my backside\n"
	.string "hurts…$"

JaggedPass_Text_AutumnIntro:
	.string "I hike to LAVARIDGE for work every\n"
	.string "day.\p"
	.string "Magma folks wave at me. They used to\n"
	.string "build our fences. Now? I don't know.$"

JaggedPass_Text_AutumnDefeat:
	.string "Hmm…\n"
	.string "Where did I slip up?$"

JaggedPass_Text_AutumnPostBattle:
	.string "What's that odd rock up the hill?\p"
	.string "TEAM MAGMA guards it. Says it's a\n"
	.string "“survey site.” Sure.$"

