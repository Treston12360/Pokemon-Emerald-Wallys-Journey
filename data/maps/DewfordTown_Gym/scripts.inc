DewfordTown_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, DewfordTown_Gym_OnTransition
	.byte 0

DewfordTown_Gym_OnTransition:
	call DewfordTown_Gym_EventScript_SetFlashLevel
	end

DewfordTown_Gym_EventScript_SetFlashLevel::
	goto_if_defeated TRAINER_BRAWLY_1, DewfordTown_Gym_EventScript_SetLightsOn
	call DewfordTown_Gym_EventScript_CountTrainersDefeated
	copyvar VAR_0x8001, VAR_0x8000
	goto_if_eq VAR_0x8000, 0, DewfordTown_Gym_EventScript_SetFlashLevel7
	goto_if_eq VAR_0x8000, 1, DewfordTown_Gym_EventScript_SetFlashLevel6
	goto_if_eq VAR_0x8000, 2, DewfordTown_Gym_EventScript_SetFlashLevel5
	goto_if_eq VAR_0x8000, 3, DewfordTown_Gym_EventScript_SetFlashLevel4
	goto_if_eq VAR_0x8000, 4, DewfordTown_Gym_EventScript_SetFlashLevel3
	goto_if_eq VAR_0x8000, 5, DewfordTown_Gym_EventScript_SetFlashLevel2
	goto DewfordTown_Gym_EventScript_SetFlashLevel1

DewfordTown_Gym_EventScript_SetLightsOn::
	setflashlevel 0
	return

@ Brightest
DewfordTown_Gym_EventScript_SetFlashLevel1::
	setflashlevel 1
	return

DewfordTown_Gym_EventScript_SetFlashLevel2::
	setflashlevel 2
	return

DewfordTown_Gym_EventScript_SetFlashLevel3::
	setflashlevel 3
	return

DewfordTown_Gym_EventScript_SetFlashLevel4::
	setflashlevel 4
	return

DewfordTown_Gym_EventScript_SetFlashLevel5::
	setflashlevel 5
	return

DewfordTown_Gym_EventScript_SetFlashLevel6::
	setflashlevel 6
	return

@ Darkest
DewfordTown_Gym_EventScript_SetFlashLevel7::
	setflashlevel 7
	return

DewfordTown_Gym_EventScript_BrightenRoom::
	call DewfordTown_Gym_EventScript_CountTrainersDefeated
	nop1
	goto_if_eq VAR_0x8000, VAR_0x8001, DewfordTown_Gym_EventScript_NoLightChange
	copyvar VAR_0x8001, VAR_0x8000
	goto_if_eq VAR_0x8000, 1, DewfordTown_Gym_EventScript_AnimateFlash1Trainer
	goto_if_eq VAR_0x8000, 2, DewfordTown_Gym_EventScript_AnimateFlash2Trainers
	goto_if_eq VAR_0x8000, 3, DewfordTown_Gym_EventScript_AnimateFlash3Trainers
	goto_if_eq VAR_0x8000, 4, DewfordTown_Gym_EventScript_AnimateFlash4Trainers
	goto_if_eq VAR_0x8000, 5, DewfordTown_Gym_EventScript_AnimateFlash5Trainers
	goto_if_eq VAR_0x8000, 6, DewfordTown_Gym_EventScript_AnimateFlash6Trainers

DewfordTown_Gym_EventScript_NoLightChange::
	return

@ NOTE: A little confusingly, a larger animateflash param value is a smaller flash radius
DewfordTown_Gym_EventScript_AnimateFlash1Trainer::
	playse SE_SWITCH
	animateflash 6
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlash2Trainers::
	playse SE_SWITCH
	animateflash 5
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlash3Trainers::
	playse SE_SWITCH
	animateflash 4
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlash4Trainers::
	playse SE_SWITCH
	animateflash 3
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlash5Trainers::
	playse SE_SWITCH
	animateflash 2
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlash6Trainers::
	playse SE_SWITCH
	animateflash 1
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_AnimateFlashFullBrightness::
	playse SE_SWITCH
	animateflash 0
	call DewfordTown_Gym_EventScript_SetFlashLevel
	return

DewfordTown_Gym_EventScript_CountTrainersDefeated::
	setvar VAR_0x8000, 0
	goto_if_not_defeated TRAINER_TAKAO, DewfordTown_Gym_EventScript_CheckJocelyn
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_CheckJocelyn::
	goto_if_not_defeated TRAINER_JOCELYN, DewfordTown_Gym_EventScript_CheckLaura
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_CheckLaura::
	goto_if_not_defeated TRAINER_LAURA, DewfordTown_Gym_EventScript_CheckBrenden
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_CheckBrenden::
	goto_if_not_defeated TRAINER_BRENDEN, DewfordTown_Gym_EventScript_CheckCristian
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_CheckCristian::
	goto_if_not_defeated TRAINER_CRISTIAN, DewfordTown_Gym_EventScript_CheckLilith
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_CheckLilith::
	goto_if_not_defeated TRAINER_LILITH, DewfordTown_Gym_EventScript_StopCountingTrainers
	addvar VAR_0x8000, 1
DewfordTown_Gym_EventScript_StopCountingTrainers::
	return

DewfordTown_Gym_EventScript_Brawly::
	trainerbattle_single TRAINER_BRAWLY_1, DewfordTown_Gym_Text_BrawlyIntro, DewfordTown_Gym_Text_BrawlyDefeat, DewfordTown_Gym_EventScript_BrawlyDefeated, NO_MUSIC
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_BULK_UP, DewfordTown_Gym_EventScript_GiveBulkUp2
	goto DewfordTown_Gym_EventScript_BrawlyRematchOffer

DewfordTown_Gym_EventScript_BrawlyDefeated::
	call DewfordTown_Gym_EventScript_AnimateFlashFullBrightness
	message DewfordTown_Gym_Text_ReceivedKnuckleBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox DewfordTown_Gym_Text_KnuckleBadgeInfoTakeThis, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_DEWFORD_GYM
	setflag FLAG_BADGE02_GET
	addvar VAR_PETALBURG_GYM_STATE, 1
	call_if_eq VAR_PETALBURG_GYM_STATE, 6, Common_EventScript_ReadyPetalburgGymForBattle
	setvar VAR_0x8008, 2
	call Common_EventScript_SetGymTrainers
	call DewfordTown_Gym_EventScript_GiveBulkUp
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox DewfordTown_Gym_Text_RegisteredBrawly, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_BRAWLY_MATCH_CALL
	setvar VAR_ROXANNE_CALL_STEP_COUNTER, 0
	setflag FLAG_ENABLE_ROXANNE_FIRST_CALL
	release
	end

DewfordTown_Gym_EventScript_GiveBulkUp::
	giveitem ITEM_TM_BULK_UP
	goto_if_eq VAR_RESULT, 0, Common_EventScript_BagIsFull
	msgbox DewfordTown_Gym_Text_ExplainBulkUp, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_BULK_UP
	call WallyJourney_EventScript_GiveGymHMs
	return

DewfordTown_Gym_EventScript_GiveBulkUp2:
	giveitem ITEM_TM_BULK_UP
	goto_if_eq VAR_RESULT, 0, Common_EventScript_ShowBagIsFull
	msgbox DewfordTown_Gym_Text_ExplainBulkUp, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_BULK_UP
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

DewfordTown_Gym_EventScript_BrawlyRematch::
	goto DewfordTown_Gym_EventScript_BrawlyRematchOffer

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
DewfordTown_Gym_EventScript_BrawlyRematchTier3::
	trainerbattle_rematch_double TRAINER_BRAWLY_1, DewfordTown_Gym_Text_BrawlyPreRematch3, DewfordTown_Gym_Text_BrawlyRematchDefeat, DewfordTown_Gym_Text_BrawlyRematchNeedTwoMons
	msgbox DewfordTown_Gym_Text_BrawlyPostRematch, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_BrawlyRematchTier4::
	trainerbattle_rematch_double TRAINER_BRAWLY_1, DewfordTown_Gym_Text_BrawlyPreRematch4, DewfordTown_Gym_Text_BrawlyRematchDefeatElite, DewfordTown_Gym_Text_BrawlyRematchNeedTwoMons
	msgbox DewfordTown_Gym_Text_BrawlyPostRematchElite, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_BrawlyRematchTier5::
	trainerbattle_rematch_double TRAINER_BRAWLY_1, DewfordTown_Gym_Text_BrawlyPreRematch5, DewfordTown_Gym_Text_BrawlyRematchDefeatElite, DewfordTown_Gym_Text_BrawlyRematchNeedTwoMons
	msgbox DewfordTown_Gym_Text_BrawlyPostRematchElite, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_Takao::
	trainerbattle_single TRAINER_TAKAO, DewfordTown_Gym_Text_TakaoIntro, DewfordTown_Gym_Text_TakaoDefeat, DewfordTown_Gym_EventScript_TakaoBrightenRoom
	msgbox DewfordTown_Gym_Text_TakaoPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_TakaoBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_Jocelyn::
	trainerbattle_single TRAINER_JOCELYN, DewfordTown_Gym_Text_JocelynIntro, DewfordTown_Gym_Text_JocelynDefeat, DewfordTown_Gym_EventScript_JocelynBrightenRoom
	msgbox DewfordTown_Gym_Text_JocelynPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_JocelynBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_Laura::
	trainerbattle_single TRAINER_LAURA, DewfordTown_Gym_Text_LauraIntro, DewfordTown_Gym_Text_LauraDefeat, DewfordTown_Gym_EventScript_LauraBrightenRoom
	msgbox DewfordTown_Gym_Text_LauraPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_LauraBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_Brenden::
	trainerbattle_single TRAINER_BRENDEN, DewfordTown_Gym_Text_BrendenIntro, DewfordTown_Gym_Text_BrendenDefeat, DewfordTown_Gym_EventScript_BrendenBrightenRoom
	msgbox DewfordTown_Gym_Text_BrendenPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_BrendenBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_Cristian::
	trainerbattle_single TRAINER_CRISTIAN, DewfordTown_Gym_Text_CristianIntro, DewfordTown_Gym_Text_CristianDefeat, DewfordTown_Gym_EventScript_CristianBrightenRoom
	msgbox DewfordTown_Gym_Text_CristianPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_CristianBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_Lilith::
	trainerbattle_single TRAINER_LILITH, DewfordTown_Gym_Text_LilithIntro, DewfordTown_Gym_Text_LilithDefeat, DewfordTown_Gym_EventScript_LilithBrightenRoom
	msgbox DewfordTown_Gym_Text_LilithPostBattle, MSGBOX_AUTOCLOSE
	end

DewfordTown_Gym_EventScript_LilithBrightenRoom::
	call DewfordTown_Gym_EventScript_BrightenRoom
	release
	end

DewfordTown_Gym_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_DEWFORD_GYM, DewfordTown_Gym_EventScript_GymGuidePostVictory
	msgbox DewfordTown_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_GymGuidePostVictory::
	msgbox DewfordTown_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE02_GET, DewfordTown_Gym_EventScript_GymStatueCertified
	goto DewfordTown_Gym_EventScript_GymStatue
	end

DewfordTown_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE02_GET, DewfordTown_Gym_EventScript_GymStatueCertified
	goto DewfordTown_Gym_EventScript_GymStatue
	end

DewfordTown_Gym_EventScript_GymStatueCertified::
	msgbox DewfordTown_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

DewfordTown_Gym_EventScript_GymStatue::
	msgbox DewfordTown_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

DewfordTown_Gym_Text_GymGuideAdvice:
	.string "Hey, how's it going, CHAMPION-\n"
	.string "bound {PLAYER}?\p"
	.string "DEWFORD's GYM LEADER BRAWLY commands\n"
	.string "FIGHTING-type POKéMON.\p"
	.string "Going against him with NORMAL-type\n"
	.string "POKéMON is asking for trouble.\p"
	.string "FIGHTING POKéMON will lay a serious\n"
	.string "smack down on NORMAL POKéMON,\l"
	.string "so be careful!\p"
	.string "What's worse, the GYM is as dark as\n"
	.string "the ocean floor.\p"
	.string "But it will get brighter after defeating\n"
	.string "the TRAINERS in your way.\p"
	.string "Hey, okay, go for it!$"

DewfordTown_Gym_Text_GymGuidePostVictory:
	.string "Whoah! It's gotten so bright in here!\p"
	.string "Here's a tip: the LEAGUE reviews\n"
	.string "every GYM each year. BRAWLY keeps\l"
	.string "the lights off and still passes!\p"
	.string "Your future is bright, too!$"

DewfordTown_Gym_Text_TakaoIntro:
	.string "I was BRAWLY's first student.\p"
	.string "Back then this GYM was a bait shed,\n"
	.string "and he was the kid gutting fish in it.\p"
	.string "Come on!$"

DewfordTown_Gym_Text_TakaoDefeat:
	.string "My fists were faster than\n"
	.string "my thinking!$"

DewfordTown_Gym_Text_TakaoPostBattle:
	.string "His dad fished these waters forty\n"
	.string "years. Then the catches dried up.\p"
	.string "BRAWLY trained instead of mourning\n"
	.string "it. That's this whole GYM, really.$"

DewfordTown_Gym_Text_JocelynIntro:
	.string "I swim here from ROUTE 106 every\n"
	.string "morning before school!\p"
	.string "You don't get to beat me after that!$"

DewfordTown_Gym_Text_JocelynDefeat:
	.string "How?! I trained every\n"
	.string "morning before school!$"

DewfordTown_Gym_Text_JocelynPostBattle:
	.string "My parents wanted me at the TRAINER'S\n"
	.string "SCHOOL in RUSTBORO.\p"
	.string "We couldn't afford it. BRAWLY charges\n"
	.string "nothing. He just says show up.\p"
	.string "So I show up. Every day.$"

DewfordTown_Gym_Text_LauraIntro:
	.string "BRAWLY got lost in GRANITE CAVE for\n"
	.string "a whole night when he was ten.\p"
	.string "That's why it's dark in here.\n"
	.string "You want to see how I learned?$"

DewfordTown_Gym_Text_LauraDefeat:
	.string "Waaah! I can't see where\n"
	.string "I went wrong!$"

DewfordTown_Gym_Text_LauraPostBattle:
	.string "Every win, the lights come up a\n"
	.string "little.\p"
	.string "He says that's how it felt walking out\n"
	.string "of the cave at dawn.$"

DewfordTown_Gym_Text_LilithIntro:
	.string "Ufufu… You think BRAWLY's all\n"
	.string "smiles and surfing?\p"
	.string "He's tougher than he looks.\n"
	.string "So am I!$"

DewfordTown_Gym_Text_LilithDefeat:
	.string "Oh, you…\n"
	.string "You walked right through me!$"

DewfordTown_Gym_Text_LilithPostBattle:
	.string "The LEAGUE offered him a GYM in\n"
	.string "SLATEPORT once. Bigger. Brighter.\p"
	.string "He said DEWFORD needed one more.\n"
	.string "He was right.$"

DewfordTown_Gym_Text_BrendenIntro:
	.string "Three winters ago, my boat went\n"
	.string "down off this island.\p"
	.string "BRAWLY swam out in the storm and\n"
	.string "dragged me in. Now I owe him this!$"

DewfordTown_Gym_Text_BrendenDefeat:
	.string "Capsized!\n"
	.string "I didn't see that swell coming!$"

DewfordTown_Gym_Text_BrendenPostBattle:
	.string "Aye. You're worthy of BRAWLY.\p"
	.string "He won't fight you like a wall.\n"
	.string "He'll come at you like a wave.$"

DewfordTown_Gym_Text_CristianIntro:
	.string "The last LEADER here quit after one\n"
	.string "season. Too far from everything.\p"
	.string "BRAWLY stayed. So I stay too!$"

DewfordTown_Gym_Text_CristianDefeat:
	.string "Grrr! Overpowered!\n"
	.string "BRAWLY will laugh at me!$"

DewfordTown_Gym_Text_CristianPostBattle:
	.string "Go after that BADGE.\p"
	.string "Kids swim over from the mainland\n"
	.string "just to train here now.\p"
	.string "Nobody did that before him.$"

DewfordTown_Gym_Text_BrawlyIntro:
	.string "I'm BRAWLY! DEWFORD's GYM LEADER!\p"
	.string "I learned to surf on these waves and\n"
	.string "to fight in the dark of GRANITE CAVE.\p"
	.string "Out here nobody comes to rescue you.\n"
	.string "So you get strong enough to rescue\l"
	.string "yourself.\p"
	.string "Show me you can do that!$"

DewfordTown_Gym_Text_BrawlyDefeat:
	.string "Whoa! You hit like a rogue wave!\p"
	.string "I didn't even see you coming.\n"
	.string "That's the point, huh?\p"
	.string "Okay, you earned it.\n"
	.string "Take this GYM BADGE!$"

DewfordTown_Gym_Text_ReceivedKnuckleBadge:
	.string "{PLAYER} received the KNUCKLE BADGE\n"
	.string "from BRAWLY.$"

DewfordTown_Gym_Text_KnuckleBadgeInfoTakeThis:
	.string "The KNUCKLE BADGE makes all POKéMON\n"
	.string "up to Level 30, even those you get in\l"
	.string "trades, obey without question.\p"
	.string "And, you'll be able to use the HM move\n"
	.string "FLASH outside of battle.\p"
	.string "If you'd like, use this TECHNICAL\n"
	.string "MACHINE, too.$"

DewfordTown_Gym_Text_ExplainBulkUp:
	.string "That TM08 contains BULK UP.\p"
	.string "It's a move that raises both ATTACK\n"
	.string "and DEFENSE stats.\p"
	.string "… … … … … …$"

DewfordTown_Gym_Text_RegisteredBrawly:
	.string "Registered GYM LEADER BRAWLY\n"
	.string "in the POKéNAV.$"

DewfordTown_Gym_Text_BrawlyPostBattle:
	.string "You caught the wave and rode it all\n"
	.string "the way in.\p"
	.string "Keep that up, and one day you'll\n"
	.string "cause a storm across HOENN!$"

DewfordTown_Gym_Text_GymStatue:
	.string "DEWFORD TOWN POKéMON GYM$"

DewfordTown_Gym_Text_GymStatueCertified:
	.string "DEWFORD TOWN POKéMON GYM\p"
	.string "BRAWLY'S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

DewfordTown_Gym_Text_BrawlyPreRematch:
	.string "BRAWLY: A wave pulls back, but it\n"
	.string "always comes back to the shore.\p"
	.string "I knew you would too.\n"
	.string "Show me how big you've gotten!$"

DewfordTown_Gym_Text_BrawlyRematchDefeat:
	.string "Wiped out!\n"
	.string "Again! What a rush!$"

DewfordTown_Gym_Text_BrawlyPostRematch:
	.string "BRAWLY: Battling's like the sea out\n"
	.string "there. You never touch the bottom.\p"
	.string "That's why I keep diving.$"

DewfordTown_Gym_Text_BrawlyRematchNeedTwoMons:
	.string "BRAWLY: A wave pulls back, but it\n"
	.string "always comes back to the shore.\p"
	.string "Oh? Only one of your POKéMON can\n"
	.string "battle?\p"
	.string "Rematches here are two at a time.\n"
	.string "Come back with at least two.$"

DewfordTown_Gym_Text_BrawlyPreRematch3:
	.string "BRAWLY: I spent all season hauling\n"
	.string "relief crates between the islands.\p"
	.string "Best training I ever did.\n"
	.string "Let's see if it shows!$"

DewfordTown_Gym_Text_BrawlyPreRematch4:
	.string "BRAWLY: The swells have been huge\n"
	.string "since the storms.\p"
	.string "I learned to ride them instead of\n"
	.string "fighting them. Let's go!$"

DewfordTown_Gym_Text_BrawlyPreRematch5:
	.string "BRAWLY: No more hiding in the dark.\n"
	.string "I turned the lights on for this one.\p"
	.string "I want to see every move coming.\n"
	.string "Bring the storm, CHAMPION!$"

DewfordTown_Gym_Text_BrawlyRematchDefeatElite:
	.string "Wiped out! What a ride!$"

DewfordTown_Gym_Text_BrawlyPostRematchElite:
	.string "BRAWLY: CHAMPION or not, you still\n"
	.string "fight like someone protecting\l"
	.string "something.\p"
	.string "Never lose that. That's what makes\n"
	.string "the wave.$"

@ Wally's Journey: growing Gym Leader rematches (see WallyJourney_ScaleLeaderParty).
DewfordTown_Gym_EventScript_BrawlyRematchOffer::
	setvar VAR_0x8004, 2
	special WallyJourney_SetupLeaderRematch
	switch VAR_RESULT
	case 1, DewfordTown_Gym_EventScript_BrawlyRematchStage1
	case 2, DewfordTown_Gym_EventScript_BrawlyRematchStage2
	case 3, DewfordTown_Gym_EventScript_BrawlyRematchStage3
	case 4, DewfordTown_Gym_EventScript_BrawlyRematchStage4
	goto DewfordTown_Gym_EventScript_BrawlyRematchStage1

DewfordTown_Gym_EventScript_BrawlyRematchStage1::
	msgbox DewfordTown_Gym_Text_BrawlyGrowOffer1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_Gym_EventScript_BrawlyRematchDecline
	cleartrainerflag TRAINER_BRAWLY_2
	trainerbattle_no_intro TRAINER_BRAWLY_2, DewfordTown_Gym_Text_BrawlyGrowDefeat1
	msgbox DewfordTown_Gym_Text_BrawlyGrowAfter1, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_BrawlyRematchStage2::
	msgbox DewfordTown_Gym_Text_BrawlyGrowOffer2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_Gym_EventScript_BrawlyRematchDecline
	cleartrainerflag TRAINER_BRAWLY_2
	trainerbattle_no_intro TRAINER_BRAWLY_2, DewfordTown_Gym_Text_BrawlyGrowDefeat2
	msgbox DewfordTown_Gym_Text_BrawlyGrowAfter2, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_BrawlyRematchStage3::
	msgbox DewfordTown_Gym_Text_BrawlyGrowOffer3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_Gym_EventScript_BrawlyRematchDecline
	cleartrainerflag TRAINER_BRAWLY_3
	trainerbattle_double TRAINER_BRAWLY_3, DewfordTown_Gym_Text_BrawlyGrowGo, DewfordTown_Gym_Text_BrawlyGrowDefeat3, DewfordTown_Gym_Text_BrawlyGrowNeedTwo
	msgbox DewfordTown_Gym_Text_BrawlyGrowAfter3, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_BrawlyRematchStage4::
	msgbox DewfordTown_Gym_Text_BrawlyGrowOffer4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_Gym_EventScript_BrawlyRematchDecline
	cleartrainerflag TRAINER_BRAWLY_4
	trainerbattle_double TRAINER_BRAWLY_4, DewfordTown_Gym_Text_BrawlyGrowGo, DewfordTown_Gym_Text_BrawlyGrowDefeat4, DewfordTown_Gym_Text_BrawlyGrowNeedTwo
	msgbox DewfordTown_Gym_Text_BrawlyGrowAfter4, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_EventScript_BrawlyRematchDecline::
	msgbox DewfordTown_Gym_Text_BrawlyPostBattle, MSGBOX_DEFAULT
	release
	end

DewfordTown_Gym_Text_BrawlyGrowGo:
	.string "Here we go!$"

DewfordTown_Gym_Text_BrawlyGrowNeedTwo:
	.string "You need two POKéMON for this\n"
	.string "battle. Come back with more.$"

DewfordTown_Gym_Text_BrawlyGrowOffer1:
	.string "BRAWLY: Whoa, {STR_VAR_1} BADGES already!\p"
	.string "I rode a bigger wave with my team.\n"
	.string "They're at {STR_VAR_2}'s level now.\p"
	.string "Want to paddle out again?$"

DewfordTown_Gym_Text_BrawlyGrowDefeat1:
	.string "Wiped out again!\n"
	.string "You're riding a monster swell.$"

DewfordTown_Gym_Text_BrawlyGrowAfter1:
	.string "Get your next BADGE, then come back.\n"
	.string "I'll be training on the rocks.$"

DewfordTown_Gym_Text_BrawlyGrowOffer2:
	.string "BRAWLY: All eight BADGES, huh?\p"
	.string "My team's nearly ELITE FOUR tough.\n"
	.string "One last wave before the LEAGUE?$"

DewfordTown_Gym_Text_BrawlyGrowDefeat2:
	.string "Totally crushed. The LEAGUE\n"
	.string "had better watch out.$"

DewfordTown_Gym_Text_BrawlyGrowAfter2:
	.string "Go get 'em at EVER GRANDE.\n"
	.string "The whole island's rooting for you.$"

DewfordTown_Gym_Text_BrawlyGrowOffer3:
	.string "BRAWLY: The CHAMPION, back in\n"
	.string "little old DEWFORD!\p"
	.string "I've got my full crew now.\n"
	.string "Rematch?$"

DewfordTown_Gym_Text_BrawlyGrowDefeat3:
	.string "You're the real deal, CHAMPION.$"

DewfordTown_Gym_Text_BrawlyGrowAfter3:
	.string "I'll keep training till I can ride\n"
	.string "a wave like you.$"

DewfordTown_Gym_Text_BrawlyGrowOffer4:
	.string "BRAWLY: You kept your title. Nice!\p"
	.string "This is my strongest team ever.\n"
	.string "Ready for the biggest wave yet?$"

DewfordTown_Gym_Text_BrawlyGrowDefeat4:
	.string "Best wipeout of my life!$"

DewfordTown_Gym_Text_BrawlyGrowAfter4:
	.string "Come surf, battle, whatever.\n"
	.string "You're always welcome here.$"

