@ Wally's Journey: Gym Leaders award the HM that their badge unlocks.
@ The badge alone lets a compatible party POKéMON use the field move
@ without learning it; the HM stays in the bag as the travel license.
@ Called after a Gym win and whenever a Gym Leader is spoken to, so a
@ full bag or an older save never leaves the player without an HM.
WallyJourney_EventScript_GiveGymHMs::
	call_if_set FLAG_BADGE01_GET, WallyJourney_EventScript_GiveHM_CUT
	call_if_set FLAG_BADGE02_GET, WallyJourney_EventScript_GiveHM_FLASH
	call_if_set FLAG_BADGE03_GET, WallyJourney_EventScript_GiveHM_ROCK_SMASH
	call_if_set FLAG_BADGE04_GET, WallyJourney_EventScript_GiveHM_STRENGTH
	call_if_set FLAG_BADGE05_GET, WallyJourney_EventScript_GiveHM_SURF
	call_if_set FLAG_BADGE06_GET, WallyJourney_EventScript_GiveHM_FLY
	call_if_set FLAG_BADGE07_GET, WallyJourney_EventScript_GiveHM_DIVE
	call_if_set FLAG_BADGE08_GET, WallyJourney_EventScript_GiveHM_WATERFALL
	return

WallyJourney_EventScript_GiveHM_CUT::
	checkitem ITEM_HM_CUT
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_CUT, MSGBOX_DEFAULT
	giveitem ITEM_HM_CUT
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_CUT
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_FLASH::
	checkitem ITEM_HM_FLASH
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_FLASH, MSGBOX_DEFAULT
	giveitem ITEM_HM_FLASH
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_FLASH
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_ROCK_SMASH::
	checkitem ITEM_HM_ROCK_SMASH
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_ROCK_SMASH, MSGBOX_DEFAULT
	giveitem ITEM_HM_ROCK_SMASH
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_ROCK_SMASH
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_STRENGTH::
	checkitem ITEM_HM_STRENGTH
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_STRENGTH, MSGBOX_DEFAULT
	giveitem ITEM_HM_STRENGTH
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_STRENGTH
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_SURF::
	checkitem ITEM_HM_SURF
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_SURF, MSGBOX_DEFAULT
	giveitem ITEM_HM_SURF
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_SURF
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_FLY::
	checkitem ITEM_HM_FLY
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_FLY, MSGBOX_DEFAULT
	giveitem ITEM_HM_FLY
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_FLY
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_DIVE::
	checkitem ITEM_HM_DIVE
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_DIVE, MSGBOX_DEFAULT
	giveitem ITEM_HM_DIVE
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_DIVE
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_GiveHM_WATERFALL::
	checkitem ITEM_HM_WATERFALL
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_HMIntro_WATERFALL, MSGBOX_DEFAULT
	giveitem ITEM_HM_WATERFALL
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	setflag FLAG_RECEIVED_HM_WATERFALL
	msgbox WallyJourney_Text_HMNoTeachingNeeded, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_HMBagFull::
	msgbox WallyJourney_Text_HMBagFull, MSGBOX_DEFAULT
	return

WallyJourney_Text_HMIntro_CUT:
	.string "The STONE BADGE also unlocks\n"
	.string "CUT out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_FLASH:
	.string "The KNUCKLE BADGE also unlocks\n"
	.string "FLASH out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_ROCK_SMASH:
	.string "The DYNAMO BADGE also unlocks\n"
	.string "ROCK SMASH out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_STRENGTH:
	.string "The HEAT BADGE also unlocks\n"
	.string "STRENGTH out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_SURF:
	.string "The BALANCE BADGE also unlocks\n"
	.string "SURF out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_FLY:
	.string "The FEATHER BADGE also unlocks\n"
	.string "FLY out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_DIVE:
	.string "The MIND BADGE also unlocks\n"
	.string "DIVE out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMIntro_WATERFALL:
	.string "The RAIN BADGE also unlocks\n"
	.string "WATERFALL out in the field.\p"
	.string "Take this HM, too. It's proof\n"
	.string "that you've earned the right.$"

WallyJourney_Text_HMNoTeachingNeeded:
	.string "You don't have to teach the move to\n"
	.string "a POKéMON. Any partner that could\l"
	.string "learn it can use it for you.$"

WallyJourney_Text_HMBagFull:
	.string "Your BAG is full. Make some room and\n"
	.string "come back for your HM.$"

@ Wally's Journey: Wally's rival battles across HOENN.
@ Each Wally is hidden until the listed badge is won, battles once,
@ then leaves for good. His dialogue grows more confident each time.
WallyJourney_EventScript_RivalVERDANTURF::
	trainerbattle_single TRAINER_WALLY_VERDANTURF, WallyJourney_Text_RivalVERDANTURFIntro, WallyJourney_Text_RivalVERDANTURFDefeat, WallyJourney_EventScript_RivalVERDANTURFWon
	msgbox WallyJourney_Text_RivalVERDANTURFPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalVERDANTURFWon::
	msgbox WallyJourney_Text_RivalVERDANTURFPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_VERDANTURF
	release
	end

WallyJourney_Text_RivalVERDANTURFIntro:
	.string "WALLY: {PLAYER}! Um… hi!\p"
	.string "I've been training every day since\n"
	.string "our battle in MAUVILLE.\p"
	.string "My uncle says the air here is good\n"
	.string "for me. I feel stronger already!\p"
	.string "W-would you battle me again?\n"
	.string "I want to see how far I've come.$"


WallyJourney_Text_RivalVERDANTURFDefeat:
	.string "I lost… but I didn't freeze up\n"
	.string "this time!$"


WallyJourney_Text_RivalVERDANTURFPost:
	.string "WALLY: Every battle teaches me\n"
	.string "something new.\p"
	.string "I'm going to keep going. Not just\n"
	.string "around VERDANTURF, either!$"


WallyJourney_EventScript_RivalLAVARIDGE::
	trainerbattle_single TRAINER_WALLY_LAVARIDGE, WallyJourney_Text_RivalLAVARIDGEIntro, WallyJourney_Text_RivalLAVARIDGEDefeat, WallyJourney_EventScript_RivalLAVARIDGEWon
	msgbox WallyJourney_Text_RivalLAVARIDGEPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalLAVARIDGEWon::
	msgbox WallyJourney_Text_RivalLAVARIDGEPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_LAVARIDGE
	release
	end

WallyJourney_Text_RivalLAVARIDGEIntro:
	.string "WALLY: {PLAYER}! I climbed all the\n"
	.string "way up here by myself!\p"
	.string "The hot springs helped me catch\n"
	.string "my breath. Now I want to test\l"
	.string "my team on you!$"


WallyJourney_Text_RivalLAVARIDGEDefeat:
	.string "You're amazing, {PLAYER}!\n"
	.string "But I'm catching up!$"


WallyJourney_Text_RivalLAVARIDGEPost:
	.string "WALLY: My POKéMON and I are\n"
	.string "learning to trust each other.\p"
	.string "Next time, I'll be ready for\n"
	.string "anything you throw at me!$"


WallyJourney_EventScript_RivalFORTREE::
	trainerbattle_single TRAINER_WALLY_FORTREE, WallyJourney_Text_RivalFORTREEIntro, WallyJourney_Text_RivalFORTREEDefeat, WallyJourney_EventScript_RivalFORTREEWon
	msgbox WallyJourney_Text_RivalFORTREEPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalFORTREEWon::
	msgbox WallyJourney_Text_RivalFORTREEPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_FORTREE
	release
	end

WallyJourney_Text_RivalFORTREEIntro:
	.string "WALLY: I heard you beat your dad,\n"
	.string "{PLAYER}. That's incredible!\p"
	.string "I've been traveling on my own now.\n"
	.string "Nobody has to look after me\l"
	.string "anymore.\p"
	.string "Let's battle! I won't go easy on you\n"
	.string "just because we're friends!$"


WallyJourney_Text_RivalFORTREEDefeat:
	.string "Wow… still not enough.\n"
	.string "But that was close!$"


WallyJourney_Text_RivalFORTREEPost:
	.string "WALLY: A year ago I couldn't even\n"
	.string "walk this far.\p"
	.string "Now I'm chasing the strongest\n"
	.string "TRAINER I know. That's you!$"


WallyJourney_EventScript_RivalLILYCOVE::
	trainerbattle_single TRAINER_WALLY_LILYCOVE, WallyJourney_Text_RivalLILYCOVEIntro, WallyJourney_Text_RivalLILYCOVEDefeat, WallyJourney_EventScript_RivalLILYCOVEWon
	msgbox WallyJourney_Text_RivalLILYCOVEPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalLILYCOVEWon::
	msgbox WallyJourney_Text_RivalLILYCOVEPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_LILYCOVE
	release
	end

WallyJourney_Text_RivalLILYCOVEIntro:
	.string "WALLY: {PLAYER}! I've been waiting\n"
	.string "for you.\p"
	.string "My team is complete now. Six\n"
	.string "POKéMON who believe in me.\p"
	.string "I'm not the kid you met on\n"
	.string "ROUTE 102 anymore. Let's go!$"


WallyJourney_Text_RivalLILYCOVEDefeat:
	.string "You're still ahead of me…\n"
	.string "but not by much!$"


WallyJourney_Text_RivalLILYCOVEPost:
	.string "WALLY: I've decided something.\p"
	.string "I'm going to challenge the\n"
	.string "POKéMON LEAGUE.\p"
	.string "And I'm going to win.$"


WallyJourney_EventScript_RivalMOSSDEEP::
	trainerbattle_single TRAINER_WALLY_MOSSDEEP, WallyJourney_Text_RivalMOSSDEEPIntro, WallyJourney_Text_RivalMOSSDEEPDefeat, WallyJourney_EventScript_RivalMOSSDEEPWon
	msgbox WallyJourney_Text_RivalMOSSDEEPPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalMOSSDEEPWon::
	msgbox WallyJourney_Text_RivalMOSSDEEPPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_MOSSDEEP
	release
	end

WallyJourney_Text_RivalMOSSDEEPIntro:
	.string "WALLY: The sea air is great,\n"
	.string "isn't it?\p"
	.string "I've battled TRAINERS all across\n"
	.string "HOENN to get ready for the LEAGUE.\p"
	.string "This time, I'm the one who's going\n"
	.string "to win, {PLAYER}!$"


WallyJourney_Text_RivalMOSSDEEPDefeat:
	.string "Hah… that was a great battle.\n"
	.string "I'm almost there.$"


WallyJourney_Text_RivalMOSSDEEPPost:
	.string "WALLY: Don't keep me waiting at\n"
	.string "the LEAGUE, {PLAYER}.\p"
	.string "I'm going to be there first.$"


WallyJourney_EventScript_RivalSOOTOPOLIS::
	trainerbattle_single TRAINER_WALLY_SOOTOPOLIS, WallyJourney_Text_RivalSOOTOPOLISIntro, WallyJourney_Text_RivalSOOTOPOLISDefeat, WallyJourney_EventScript_RivalSOOTOPOLISWon
	msgbox WallyJourney_Text_RivalSOOTOPOLISPost, MSGBOX_AUTOCLOSE
	end

WallyJourney_EventScript_RivalSOOTOPOLISWon::
	msgbox WallyJourney_Text_RivalSOOTOPOLISPost, MSGBOX_DEFAULT
	setflag FLAG_HIDE_WALLY_RIVAL_SOOTOPOLIS
	release
	end

WallyJourney_Text_RivalSOOTOPOLISIntro:
	.string "WALLY: So you earned all eight\n"
	.string "BADGES. Me too.\p"
	.string "That crisis here… I saw you face it\n"
	.string "head-on. I won't forget that.\p"
	.string "One more battle before VICTORY\n"
	.string "ROAD. Show me everything!$"


WallyJourney_Text_RivalSOOTOPOLISDefeat:
	.string "That's the {PLAYER} I know!\n"
	.string "I'll be waiting at the top.$"


WallyJourney_Text_RivalSOOTOPOLISPost:
	.string "WALLY: The next time we battle, it\n"
	.string "won't be practice.\p"
	.string "I'll see you at the POKéMON LEAGUE.$"



@ Wally's Journey: postgame Jirachi. After the first Elite Four win, Prof. Cozmo
@ tells the player about a star-shaped POKéMON sleeping deep in METEOR FALLS.
@ If the player knocks it out or runs, it returns the next time they visit.
WallyJourney_EventScript_CozmoJirachiQuest::
	goto_if_set FLAG_CAUGHT_JIRACHI_WALLY, Common_EventScript_NopReturn
	goto_if_set FLAG_JIRACHI_QUEST_STARTED, WallyJourney_EventScript_CozmoJirachiReminder
	msgbox WallyJourney_Text_CozmoJirachiStory, MSGBOX_DEFAULT
	setflag FLAG_JIRACHI_QUEST_STARTED
	clearflag FLAG_HIDE_METEOR_FALLS_JIRACHI
	release
	end

WallyJourney_EventScript_CozmoJirachiReminder::
	msgbox WallyJourney_Text_CozmoJirachiReminder, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_Jirachi::
	lock
	faceplayer
	msgbox WallyJourney_Text_JirachiAsleep, MSGBOX_DEFAULT
	waitse
	playmoncry SPECIES_JIRACHI, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_JIRACHI, 70
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, WallyJourney_EventScript_JirachiCaught
	@ Knocked out or escaped: it vanishes for now, but returns on the next visit.
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreenswapbuffers FADE_FROM_BLACK
	msgbox WallyJourney_Text_JirachiVanished, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_JirachiCaught::
	setflag FLAG_CAUGHT_JIRACHI_WALLY
	setflag FLAG_HIDE_METEOR_FALLS_JIRACHI
	release
	end

WallyJourney_Text_CozmoJirachiStory:
	.string "PROF. COZMO: Oh, the new CHAMPION!\p"
	.string "I've been studying the METEORITE\n"
	.string "again. It's been glowing lately.\p"
	.string "Old legends say a star-shaped\n"
	.string "POKéMON sleeps for a thousand\l"
	.string "years and grants wishes.\p"
	.string "Hikers swear they saw a soft light\n"
	.string "deep inside METEOR FALLS, past the\l"
	.string "underground lake.\p"
	.string "If anyone can find it, it's you!$"

WallyJourney_Text_CozmoJirachiReminder:
	.string "PROF. COZMO: Any luck in METEOR\n"
	.string "FALLS? The light was seen past\l"
	.string "the underground lake.$"

WallyJourney_Text_JirachiAsleep:
	.string "A small POKéMON is dozing here,\n"
	.string "glowing softly…\p"
	.string "It opened its eyes!$"

WallyJourney_Text_JirachiVanished:
	.string "JIRACHI drifted away in a shimmer\n"
	.string "of light…\p"
	.string "It might come back here again.$"
