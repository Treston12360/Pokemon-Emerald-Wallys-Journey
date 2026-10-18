AquaHideout_B2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, AquaHideout_B2F_OnTransition
	.byte 0

AquaHideout_B2F_OnTransition:
	call_if_set FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, AquaHideout_B2F_EventScript_PreventMattNoticing
	end

AquaHideout_B2F_EventScript_PreventMattNoticing::
	setvar VAR_TEMP_1, 1
	return

AquaHideout_B2F_EventScript_MattNoticePlayer::
	lockall
	setvar VAR_0x8008, LOCALID_AQUA_HIDEOUT_MATT
	playse SE_PIN
	applymovement VAR_0x8008, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement VAR_0x8008, Common_Movement_FacePlayer
	waitmovement 0
	setvar VAR_TEMP_1, 1
	releaseall
	end

AquaHideout_B2F_EventScript_Matt::
	trainerbattle_single TRAINER_MATT, AquaHideout_B2F_Text_MattIntro, AquaHideout_B2F_Text_MattDefeat, AquaHideout_B2F_EventScript_SubmarineEscape
	msgbox AquaHideout_B2F_Text_MattPostBattle, MSGBOX_DEFAULT
	release
	end

AquaHideout_B2F_EventScript_SubmarineEscape::
	setvar VAR_0x8008, LOCALID_AQUA_HIDEOUT_MATT
	setvar VAR_0x8009, LOCALID_AQUA_HIDEOUT_SUBMARINE
	applymovement VAR_0x8008, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	delay 20
	applymovement VAR_0x8008, Common_Movement_FacePlayer
	waitmovement 0
	msgbox AquaHideout_B2F_Text_OurBossGotThroughHisPreparations, MSGBOX_DEFAULT
	closemessage
	applymovement VAR_0x8008, Common_Movement_WalkInPlaceFasterLeft
	applymovement VAR_0x8009, AquaHideout_B2F_Movement_SumbarineDepartLeft
	waitmovement 0
	removeobject VAR_0x8009
	delay 20
	applymovement VAR_0x8008, Common_Movement_FacePlayer
	waitmovement 0
	msgbox AquaHideout_B2F_Text_MattPostBattle, MSGBOX_DEFAULT
	setflag FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE
	setflag FLAG_HIDE_LILYCOVE_CITY_AQUA_GRUNTS
	release
	end

AquaHideout_B2F_Movement_SumbarineDepartLeft:
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

@ Unused
AquaHideout_B2F_Movement_SumbarineDepartRight:
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

AquaHideout_B2F_EventScript_Grunt4::
	trainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_4, AquaHideout_B2F_Text_Grunt4Intro, AquaHideout_B2F_Text_Grunt4Defeat, AquaHideout_B2F_EventScript_Grunt4Defeated
	msgbox AquaHideout_B2F_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

AquaHideout_B2F_EventScript_Grunt4Defeated::
	msgbox AquaHideout_B2F_Text_Grunt4PostBattle, MSGBOX_DEFAULT
	release
	end

AquaHideout_B2F_EventScript_Grunt6::
	trainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_6, AquaHideout_B2F_Text_Grunt6Intro, AquaHideout_B2F_Text_Grunt6Defeat
	msgbox AquaHideout_B2F_Text_Grunt6PostBattle, MSGBOX_AUTOCLOSE
	end

AquaHideout_B2F_EventScript_Grunt8::
	trainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_8, AquaHideout_B2F_Text_Grunt8Intro, AquaHideout_B2F_Text_Grunt8Defeat
	msgbox AquaHideout_B2F_Text_Grunt8PostBattle, MSGBOX_AUTOCLOSE
	end

AquaHideout_B2F_Text_MattIntro:
	.string "Hehehe…\p"
	.string "You got here fast. We\n"
	.string "underestimated you!\p"
	.string "I'm a cut above the GRUNTS. No\n"
	.string "stalling. I'll pulverize you!$"

AquaHideout_B2F_Text_MattDefeat:
	.string "Hehehe…\n"
	.string "So I lost too. Huh.$"

AquaHideout_B2F_Text_OurBossGotThroughHisPreparations:
	.string "Hehehe!\p"
	.string "While I was toying with you, our BOSS\n"
	.string "got through his preparations!$"

AquaHideout_B2F_Text_MattPostBattle:
	.string "Hehehe! The BOSS already left for\n"
	.string "some cave under the sea!\p"
	.string "Chase him if you like. Search the\n"
	.string "wide sea beyond LILYCOVE.\p"
	.string "Think you'll find it? Hehehe!$"

AquaHideout_B2F_Text_Grunt4Intro:
	.string "Wahahah! I've been waiting forever!\n"
	.string "You owe me a battle!$"

AquaHideout_B2F_Text_Grunt4Defeat:
	.string "Waited all that time…\n"
	.string "for this?$"

AquaHideout_B2F_Text_Grunt4PostBattle:
	.string "BOSS…\n"
	.string "Is this good enough?\p"
	.string "…I signed up to save the sea.\n"
	.string "Stealing a submarine isn't that.$"

AquaHideout_B2F_Text_Grunt6Intro:
	.string "Our warp panels!\p"
	.string "You have no idea where you are,\n"
	.string "do you?\p"
	.string "Confuse them, tire them out, then\n"
	.string "strike! That's the plan!$"

AquaHideout_B2F_Text_Grunt6Defeat:
	.string "You're not even tired!\n"
	.string "How?!$"

AquaHideout_B2F_Text_Grunt6PostBattle:
	.string "Oh no… Where did I put our spare\n"
	.string "POKé BALLS?\p"
	.string "If I lose them, the BOSS will\n"
	.string "chew me out…$"

AquaHideout_B2F_Text_Grunt8Intro:
	.string "My first lesson in TEAM AQUA:\n"
	.string "these warp panels.\p"
	.string "Not reef surveys. Not cleanups.\n"
	.string "Warp panels.$"

AquaHideout_B2F_Text_Grunt8Defeat:
	.string "I was thinking about the\n"
	.string "panels instead of you…$"

AquaHideout_B2F_Text_Grunt8PostBattle:
	.string "Some of us told ARCHIE the old\n"
	.string "texts don't say “control.”\p"
	.string "He didn't want to hear it.$"
