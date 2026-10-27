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
	call_if_set FLAG_BADGE06_GET, WallyJourney_EventScript_GiveZPowerRing
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
	.string "The BADGE and HM are all you\n"
	.string "need to use this field move.\p"
	.string "No POKéMON needs to learn it\n"
	.string "or even be able to learn it.$"

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

@ Spoken at the north end of the FIERY PATH (owner-approved 2026-10-26).
WallyJourney_Text_RivalVERDANTURFIntro:
	.string "WALLY: {PLAYER}! You made it through!\p"
	.string "I came in from the other side.\n"
	.string "It's so hot in here…\p"
	.string "I heard you beat WATTSON! I've been\n"
	.string "training every day since our battle\l"
	.string "in MAUVILLE.\p"
	.string "Will you battle me again? I want to\n"
	.string "see how far I've come.$"


WallyJourney_Text_RivalVERDANTURFDefeat:
	.string "I lost… but I didn't freeze up\n"
	.string "this time!$"


WallyJourney_Text_RivalVERDANTURFPost:
	.string "WALLY: Every battle teaches me\n"
	.string "something new.\p"
	.string "I'm going to keep going. Next time,\n"
	.string "I'll be even stronger!$"


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
	.string "WALLY: {PLAYER}. There you are.\p"
	.string "Mom said you beat NORMAN.\n"
	.string "Not bad. Really.\p"
	.string "I beat WINONA, too.\n"
	.string "She was clever, but I saw\l"
	.string "through her strategy.\p"
	.string "Don't treat me like that\n"
	.string "kid from PETALBURG.\p"
	.string "I'm not sure you've noticed\n"
	.string "how strong I've become.$"


WallyJourney_Text_RivalFORTREEDefeat:
	.string "Tch… Still not enough.\n"
	.string "That was close, though.$"


WallyJourney_Text_RivalFORTREEPost:
	.string "WALLY: You won again.\p"
	.string "I used to be grateful just\n"
	.string "to stand beside you.\p"
	.string "Now I want to stand above\n"
	.string "you on the rankings.\p"
	.string "Enjoy being ahead.\n"
	.string "It won't last.$"


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
	.string "WALLY: You're finally here.\p"
	.string "I've built this team to win.\n"
	.string "Every move has a purpose.\p"
	.string "You helped me once, sure.\n"
	.string "But that doesn't make you\l"
	.string "my teacher forever.\p"
	.string "Let's see whether you've\n"
	.string "managed to keep up.$"


WallyJourney_Text_RivalLILYCOVEDefeat:
	.string "You're still ahead of me…\n"
	.string "but not by much!$"


WallyJourney_Text_RivalLILYCOVEPost:
	.string "WALLY: WALLACE watched me\n"
	.string "battle the other day.\p"
	.string "He asked what I still\n"
	.string "needed to prove.\p"
	.string "I almost laughed.\n"
	.string "Isn't winning the answer?\p"
	.string "Anyway, I'll see you\n"
	.string "at the LEAGUE.$"


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
	.string "WALLY: Eight BADGES. Already.\p"
	.string "I'm heading for the\n"
	.string "ELITE FOUR next.\p"
	.string "You've made progress too.\n"
	.string "Just not quite as fast.\p"
	.string "The weather is turning.\n"
	.string "I'll help where I can.\p"
	.string "Then we battle. Let's see\n"
	.string "if you can surprise me.$"


WallyJourney_Text_RivalMOSSDEEPDefeat:
	.string "Close. Really close.\n"
	.string "Next time it won't be.$"


WallyJourney_Text_RivalMOSSDEEPPost:
	.string "WALLY: WALLACE keeps asking\n"
	.string "why I want to be CHAMPION.\p"
	.string "Because I know I can win.\n"
	.string "I haven't proved it yet.\p"
	.string "He says strength is more\n"
	.string "than collecting victories.\p"
	.string "I'll show him what I can do\n"
	.string "when I reach the LEAGUE.$"


@ Wally's Journey (owner design): right before the 8th Gym, Wally stops the player at
@ the Gym door and beats them, then flies to Ever Grande for his title match. Losing
@ doesn't white out (FLAG_WALLY_NO_WHITEOUT); winning is possible and gets its own lines.
WallyJourney_EventScript_SootopolisWallyBattle::
	msgbox WallyJourney_Text_SootopolisWallyCalls, MSGBOX_DEFAULT
	closemessage
	@ The Gym stands on an island in the lake: Wally surfs up from below the screen, hops
	@ ashore and walks up to the player.
	clearflag FLAG_HIDE_WALLY_RIVAL_SOOTOPOLIS
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	setvar VAR_0x8009, LOCALID_SOOTOPOLIS_RIVAL_WALLY
	setobjectxyperm LOCALID_SOOTOPOLIS_RIVAL_WALLY, 31, 40
	addobject LOCALID_SOOTOPOLIS_RIVAL_WALLY
	special WallyJourney_WallySurfStart
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_SootopolisSurfIn
	waitmovement 0
	special WallyJourney_WallySurfLeaveBlob
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_HopUp
	waitmovement 0
	special WallyJourney_WallySurfEnd
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_SootopolisWalkToPlayer
	waitmovement 0
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalSOOTOPOLISIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_SOOTOPOLIS, WallyJourney_Text_RivalSOOTOPOLISDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	special HealPlayerParty
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_SootopolisPlayerWon
	msgbox WallyJourney_Text_SootopolisWallyWon, MSGBOX_DEFAULT
	goto WallyJourney_EventScript_SootopolisWallyLeaves

WallyJourney_EventScript_SootopolisPlayerWon::
	msgbox WallyJourney_Text_RivalSOOTOPOLISPost, MSGBOX_DEFAULT
WallyJourney_EventScript_SootopolisWallyLeaves::
	closemessage
	@ He walks back to the shore, hops onto his POKéMON and surfs off below the screen.
	setvar VAR_0x8009, LOCALID_SOOTOPOLIS_RIVAL_WALLY
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_SootopolisWalkToShore
	waitmovement 0
	special WallyJourney_WallySurfBlobBelow
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_HopDown
	waitmovement 0
	special WallyJourney_WallySurfMount
	applymovement LOCALID_SOOTOPOLIS_RIVAL_WALLY, WallyJourney_Movement_SootopolisSurfOut
	waitmovement 0
	special WallyJourney_WallySurfEnd
	removeobject LOCALID_SOOTOPOLIS_RIVAL_WALLY
	setflag FLAG_HIDE_WALLY_RIVAL_SOOTOPOLIS
	clearflag FLAG_WALLY_NO_WHITEOUT
	fadedefaultbgm
	applymovement LOCALID_PLAYER, Common_Movement_FaceUp
	waitmovement 0
	return

@ Sootopolis: Wally starts on the lake at (31,40), below the screen; the island's south shore
@ is (31,36) and the player stands at (31,33) in front of the Gym door.
WallyJourney_Movement_SootopolisSurfIn:
	walk_up
	walk_up
	walk_up
	step_end

WallyJourney_Movement_HopUp:
	jump_up
	step_end

WallyJourney_Movement_SootopolisWalkToPlayer:
	walk_up
	walk_up
	face_up
	step_end

WallyJourney_Movement_SootopolisWalkToShore:
	walk_down
	walk_down
	step_end

WallyJourney_Movement_HopDown:
	jump_down
	step_end

WallyJourney_Movement_SootopolisSurfOut:
	walk_down
	walk_down
	walk_down
	step_end

WallyJourney_Movement_WallyWalkUp2:
	walk_up
	walk_up
	step_end

WallyJourney_Movement_WallyWalkDown2:
	walk_down
	walk_down
	step_end

WallyJourney_Text_SootopolisWallyCalls:
	.string "WALLY: {PLAYER}! Hold up.$"

WallyJourney_Text_SootopolisWallyWon:
	.string "WALLY: So that's the gap between\n"
	.string "us now.\p"
	.string "I told you I'd catch up. I didn't\n"
	.string "think I'd pass you this fast.\p"
	.string "Watch the broadcast tonight.\n"
	.string "When I beat WALLACE, you'll know\l"
	.string "exactly where we stand.$"

WallyJourney_Text_RivalSOOTOPOLISIntro:
	.string "WALLY: The storm's over.\n"
	.string "Everyone I got into the shelters\l"
	.string "walked out on their own.\p"
	.string "I cleared the ELITE FOUR last week.\n"
	.string "Then the sky went black, and I came\l"
	.string "back to help.\p"
	.string "WALLACE just left for EVER GRANDE.\n"
	.string "Our title match is tonight.\p"
	.string "One battle before I go. I'm saving\n"
	.string "MEGA EVOLUTION for him.\p"
	.string "And don't expect me to go easy just\n"
	.string "because you watched me catch RALTS.$"


WallyJourney_Text_RivalSOOTOPOLISDefeat:
	.string "WALLY: …You still beat me.\n"
	.string "Fine. Better you than WALLACE.$"


WallyJourney_Text_RivalSOOTOPOLISPost:
	.string "WALLY: Enjoy it. Tonight I fight\n"
	.string "at full strength.\p"
	.string "When I take that title, you'll be\n"
	.string "the one chasing me.$"



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


@ Wally's Journey: the Z-Power Ring arrives with the sixth badge (owner decision).
@ Handed over by whichever Gym Leader the player talks to first after badge 6.
WallyJourney_EventScript_GiveZPowerRing::
	checkitem ITEM_Z_POWER_RING
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	msgbox WallyJourney_Text_ZRingIntro, MSGBOX_DEFAULT
	giveitem ITEM_Z_POWER_RING
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HMBagFull
	msgbox WallyJourney_Text_ZRingExplain, MSGBOX_DEFAULT
	return

WallyJourney_Text_ZRingIntro:
	.string "One more thing. A TRAINER from the\n"
	.string "ALOLA region left this with the\l"
	.string "LEAGUE for whoever earned six BADGES.$"

WallyJourney_Text_ZRingExplain:
	.string "Hold a Z-CRYSTAL and the ring lets\n"
	.string "your POKéMON unleash a Z-MOVE once\l"
	.string "per battle.\p"
	.string "Z-CRYSTALS are said to be hidden all\n"
	.string "over HOENN. You can switch Z-MOVES\l"
	.string "off in the OPTIONS if you prefer.$"


@ Wally's Journey (owner design): Wally battles the player the first time they try to
@ enter the 8th Gym. After the 8th badge, Steven stops the player outside the Gym and
@ takes them to JUAN's house, where they watch Wally beat Wallace live. Juan taught Wallace; Wallace guided Wally; Steven guided you.
@ Unskippable: the Wally trigger sits on the only tile in front of the Gym door; the
@ broadcast starts from SootopolisCity_OnFrame when leaving the Gym with the badge.
WallyJourney_EventScript_SootopolisGymDoor::
	goto_if_set FLAG_WALLY_SOOTOPOLIS_DOOR_DONE, Common_EventScript_NopReturn
	lockall
	call WallyJourney_EventScript_SootopolisWallyBattle
	setflag FLAG_WALLY_SOOTOPOLIS_DOOR_DONE
	releaseall
	end

WallyJourney_EventScript_StevenStopsAtGym::
	lockall
	setvar VAR_WALLY_RIVAL_PENDING, 0
	applymovement LOCALID_SOOTOPOLIS_STEVEN, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_SOOTOPOLIS_STEVEN, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_FaceLeft
	waitmovement 0
	msgbox WallyJourney_Text_StevenFollowMe, MSGBOX_DEFAULT
	closemessage
	setvar VAR_WALLY_BROADCAST_STATE, 2
	clearflag FLAG_HIDE_JUANS_HOUSE_JUAN
	clearflag FLAG_HIDE_JUANS_HOUSE_STEVEN
	warp MAP_SOOTOPOLIS_CITY_HOUSE7, 7, 3
	waitstate
	releaseall
	end

WallyJourney_EventScript_JuansHouseBroadcast::
	lockall
	setvar VAR_WALLY_BROADCAST_STATE, 3
	msgbox WallyJourney_Text_StevenWeLetOurselvesIn, MSGBOX_DEFAULT
	applymovement LOCALID_JUANS_HOUSE_JUAN, Common_Movement_FaceDown
	waitmovement 0
	msgbox WallyJourney_Text_JuanComeQuickly, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_FaceUp
	waitmovement 0
	applymovement LOCALID_JUANS_HOUSE_JUAN, Common_Movement_FaceUp
	waitmovement 0
	fadescreen FADE_TO_BLACK
	playbgm MUS_ENCOUNTER_CHAMPION, FALSE
	msgbox WallyJourney_Text_BroadcastAnnouncer, MSGBOX_DEFAULT
	msgbox WallyJourney_Text_BroadcastWallace, MSGBOX_DEFAULT
	msgbox WallyJourney_Text_BroadcastWally, MSGBOX_DEFAULT
	closemessage
	fadedefaultbgm
	fadescreen FADE_FROM_BLACK
	applymovement LOCALID_JUANS_HOUSE_JUAN, Common_Movement_FaceRight
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_FaceLeft
	waitmovement 0
	msgbox WallyJourney_Text_JuanAfterBroadcast, MSGBOX_DEFAULT
	msgbox WallyJourney_Text_StevenAfterBroadcast, MSGBOX_DEFAULT
	msgbox WallyJourney_Text_JuanSeeYouAtGym, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_JUANS_HOUSE_JUAN
	removeobject LOCALID_JUANS_HOUSE_STEVEN
	setflag FLAG_HIDE_JUANS_HOUSE_JUAN
	setflag FLAG_HIDE_JUANS_HOUSE_STEVEN
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_STEVEN
	fadescreenswapbuffers FADE_FROM_BLACK
	releaseall
	end

WallyJourney_Text_WallaceMustReturn:
	.string "WALLACE: Now, forgive me, but I must\n"
	.string "fly back to EVER GRANDE at once.\p"
	.string "A challenger has made it through the\n"
	.string "ELITE FOUR. My title is on the line\l"
	.string "tonight.\p"
	.string "Someone I've been keeping an eye on\n"
	.string "for quite a while.\p"
	.string "…Perhaps you'll guess who.$"

WallyJourney_Text_StevenFollowMe:
	.string "STEVEN: {PLAYER}! Congratulations\n"
	.string "on the RAIN BADGE.\p"
	.string "I've been looking for you. JUAN\n"
	.string "headed home the moment you won.\p"
	.string "You're going to want to see this.\n"
	.string "Follow me. Quickly!$"

WallyJourney_Text_StevenWeLetOurselvesIn:
	.string "STEVEN: JUAN, I brought {PLAYER}.\n"
	.string "Has it started?$"

WallyJourney_Text_JuanComeQuickly:
	.string "JUAN: Ah, my newest BADGE holder.\p"
	.string "Come, quickly. My former student is\n"
	.string "defending his title.\p"
	.string "It is the final POKéMON for both.$"

WallyJourney_Text_BroadcastAnnouncer:
	.string "ANNOUNCER: …live from the\n"
	.string "CHAMPION's room at EVER GRANDE!\p"
	.string "It's down to the last POKéMON!\n"
	.string "CHAMPION WALLACE sends out MILOTIC!\p"
	.string "The challenger, WALLY, answers with\n"
	.string "GARDEVOIR. No MEGA EVOLUTION!\p"
	.string "He's holding his trump card back,\n"
	.string "even now!\p"
	.string "MILOTIC lands a SCALD! GARDEVOIR\n"
	.string "is hanging on!\p"
	.string "MOONBLAST! MILOTIC is down!\p"
	.string "Ladies and gentlemen, HOENN has\n"
	.string "a new CHAMPION!\p"
	.string "WALLY, of PETALBURG and VERDANTURF!$"

WallyJourney_Text_BroadcastWallace:
	.string "WALLACE: Magnificent.\p"
	.string "When we first met, you asked me how\n"
	.string "to stop being afraid.\p"
	.string "I never answered. You found it on\n"
	.string "your own. All I did was walk beside\l"
	.string "you for a while.\p"
	.string "HOENN is in good hands.$"

WallyJourney_Text_BroadcastWally:
	.string "WALLY: Thank you, WALLACE.\n"
	.string "You gave me a real battle.\p"
	.string "I used to watch champions\n"
	.string "and wonder how they did it.\p"
	.string "Now I know. You just have\n"
	.string "to be stronger than everyone.\p"
	.string "There's still one TRAINER\n"
	.string "who thinks they know me.\p"
	.string "{PLAYER}, I'll be waiting.\n"
	.string "Try not to disappoint me.$"

WallyJourney_Text_JuanAfterBroadcast:
	.string "JUAN: …So WALLACE has been dethroned.\p"
	.string "It was I who taught WALLACE, long\n"
	.string "ago. He was proud, and impatient,\l"
	.string "and brilliant.\p"
	.string "WALLACE really must have seen\n"
	.string "something special in that boy.\p"
	.string "The very thing I once saw in him.$"

WallyJourney_Text_StevenAfterBroadcast:
	.string "STEVEN: …Like I saw in you, {PLAYER}.\p"
	.string "WALLY's already at the top.\n"
	.string "He's waiting for you.$"

WallyJourney_Text_JuanSeeYouAtGym:
	.string "JUAN: You hold every BADGE in HOENN.\p"
	.string "VICTORY ROAD lies to the east of\n"
	.string "EVER GRANDE. Go, and climb it.$"



@ Wally's Journey: after the Hall of Fame, May/Brendan asks for one last real
@ battle in Birch's lab, then explains they've chosen research as well as battling.
WallyJourney_EventScript_RivalFinal::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, WallyJourney_EventScript_RivalFinalBrendan
	msgbox WallyJourney_Text_RivalFinalMayIntro, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_STARTER_MON, 0, WallyJourney_EventScript_RivalFinal_MAY_TREECKO
	goto_if_eq VAR_STARTER_MON, 1, WallyJourney_EventScript_RivalFinal_MAY_TORCHIC
	goto WallyJourney_EventScript_RivalFinal_MAY_MUDKIP
	end

WallyJourney_EventScript_RivalFinalBrendan::
	msgbox WallyJourney_Text_RivalFinalBrendanIntro, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_STARTER_MON, 0, WallyJourney_EventScript_RivalFinal_BRENDAN_TREECKO
	goto_if_eq VAR_STARTER_MON, 1, WallyJourney_EventScript_RivalFinal_BRENDAN_TORCHIC
	goto WallyJourney_EventScript_RivalFinal_BRENDAN_MUDKIP
	end

WallyJourney_EventScript_RivalFinalAfter::
	setflag FLAG_WALLY_RIVAL_FINAL_DONE
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, WallyJourney_EventScript_RivalFinalAfterBrendan
	msgbox WallyJourney_Text_RivalFinalMayAfter, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_RivalFinalAfterBrendan::
	msgbox WallyJourney_Text_RivalFinalBrendanAfter, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_RivalFinal_MAY_TREECKO::
	trainerbattle_no_intro TRAINER_MAY_FINAL_TREECKO, WallyJourney_Text_RivalFinalMayDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_EventScript_RivalFinal_MAY_TORCHIC::
	trainerbattle_no_intro TRAINER_MAY_FINAL_TORCHIC, WallyJourney_Text_RivalFinalMayDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_EventScript_RivalFinal_MAY_MUDKIP::
	trainerbattle_no_intro TRAINER_MAY_FINAL_MUDKIP, WallyJourney_Text_RivalFinalMayDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_EventScript_RivalFinal_BRENDAN_TREECKO::
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_TREECKO, WallyJourney_Text_RivalFinalBrendanDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_EventScript_RivalFinal_BRENDAN_TORCHIC::
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_TORCHIC, WallyJourney_Text_RivalFinalBrendanDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_EventScript_RivalFinal_BRENDAN_MUDKIP::
	trainerbattle_no_intro TRAINER_BRENDAN_FINAL_MUDKIP, WallyJourney_Text_RivalFinalBrendanDefeat
	goto WallyJourney_EventScript_RivalFinalAfter
	end

WallyJourney_Text_RivalFinalMayIntro:
	.string "MAY: {PLAYER}{KUN}! The CHAMPION, in\n"
	.string "my dad's lab!\p"
	.string "I've been thinking about what I want\n"
	.string "to do next. Before I decide…\p"
	.string "One more battle. A real one.\n"
	.string "No holding back!$"

WallyJourney_Text_RivalFinalMayDefeat:
	.string "…Yeah. That's what I thought.\n"
	.string "You're the real deal.$"

WallyJourney_Text_RivalFinalMayAfter:
	.string "MAY: I've decided. I'm staying on as\n"
	.string "Dad's field partner.\p"
	.string "The data we logged during the crisis\n"
	.string "is helping the coast recover. That\l"
	.string "felt bigger than any BADGE.\p"
	.string "I'll still battle! I'm just going to\n"
	.string "do it everywhere, for science.\p"
	.string "Thanks, {PLAYER}{KUN}. For being the\n"
	.string "rival I needed.$"

WallyJourney_Text_RivalFinalBrendanIntro:
	.string "BRENDAN: Hey, CHAMPION.\p"
	.string "I've been doing a lot of thinking\n"
	.string "since SOOTOPOLIS.\p"
	.string "Before I tell you what I decided,\n"
	.string "battle me. Full strength.$"

WallyJourney_Text_RivalFinalBrendanDefeat:
	.string "Heh. Of course.\n"
	.string "Good. I needed to see that.$"

WallyJourney_Text_RivalFinalBrendanAfter:
	.string "BRENDAN: I'm going to work with my\n"
	.string "dad. Officially.\p"
	.string "Tracking migrations, helping the\n"
	.string "habitats heal. It matters.\p"
	.string "Doesn't mean I'll stop battling.\n"
	.string "Someone has to keep you sharp.\p"
	.string "See you around, {PLAYER}.$"


@ Wally's Journey: during the Sootopolis crisis Wally is outside the Pokémon
@ Center getting residents to shelter (continuity bible #37).
WallyJourney_EventScript_SootopolisCrisisWally::
	lock
	faceplayer
	msgbox WallyJourney_Text_SootopolisCrisisWally, MSGBOX_DEFAULT
	release
	end

WallyJourney_Text_SootopolisCrisisWally:
	.string "WALLY: {PLAYER}! Keep moving, I've\n"
	.string "got this side of town.\p"
	.string "Everyone's going into the POKéMON\n"
	.string "CENTER, it's the strongest building.\p"
	.string "Go! Whatever's out there, you're\n"
	.string "the one who can reach it!$"

@ Wally's Journey: Battle Frontier "Gimmick Collector". For 48 BP, a random
@ Mega Stone or Z-Crystal the player doesn't own yet. A repeatable way to fill
@ gaps; every item here is also hidden somewhere in Hoenn.
WallyJourney_EventScript_GimmickCollector::
	lock
	faceplayer
	msgbox WallyJourney_Text_CollectorOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, WallyJourney_EventScript_CollectorBye
	specialvar VAR_TEMP_1, GetFrontierBattlePoints
	goto_if_lt VAR_TEMP_1, 48, WallyJourney_EventScript_CollectorNoBP
	special WallyJourney_PickFrontierGimmickItem
	goto_if_eq VAR_0x8005, ITEM_NONE, WallyJourney_EventScript_CollectorHaveAll
	checkitemspace VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setvar VAR_0x8004, 48
	special TakeFrontierBattlePoints
	giveitem VAR_0x8005
	msgbox WallyJourney_Text_CollectorThanks, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_CollectorBye::
	msgbox WallyJourney_Text_CollectorBye, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_CollectorNoBP::
	msgbox WallyJourney_Text_CollectorNoBP, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_CollectorHaveAll::
	msgbox WallyJourney_Text_CollectorHaveAll, MSGBOX_DEFAULT
	release
	end

WallyJourney_Text_CollectorOffer:
	.string "I dig up MEGA STONES and Z-CRYSTALS\n"
	.string "all over HOENN.\p"
	.string "For 48 BATTLE POINTS, I'll trade you\n"
	.string "one you don't have yet. Deal?$"

WallyJourney_Text_CollectorThanks:
	.string "Pleasure doing business. Come back\n"
	.string "when you've earned more BP!$"

WallyJourney_Text_CollectorBye:
	.string "Suit yourself. They're out there in\n"
	.string "the caves if you'd rather dig.$"

WallyJourney_Text_CollectorNoBP:
	.string "You'll need 48 BATTLE POINTS.\n"
	.string "Win a few more rounds first.$"

WallyJourney_Text_CollectorHaveAll:
	.string "Whoa. You already have every one\n"
	.string "I could find. Impressive!$"

@ Wally's Journey: Wally team-up at Mt. Pyre (the League asked him to watch it).
@ Only fires while Team Aqua holds the summit and before they take the Orbs.
WallyJourney_EventScript_PyreTeamUp::
	goto_if_set FLAG_HIDE_MT_PYRE_SUMMIT_TEAM_AQUA, WallyJourney_EventScript_PyreTeamUpNotYet
	goto_if_ne VAR_MT_PYRE_STATE, 0, WallyJourney_EventScript_PyreTeamUpNotYet
	lockall
	clearflag FLAG_HIDE_MT_PYRE_SUMMIT_WALLY
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	setvar VAR_0x8009, LOCALID_MT_PYRE_SUMMIT_WALLY
	call WallyJourney_EventScript_WallyWalksUp
	msgbox WallyJourney_Text_PyreWallyArrives, MSGBOX_DEFAULT
	closemessage
	special SavePlayerParty
	fadescreen FADE_TO_BLACK
	special ChooseHalfPartyForBattle
	goto_if_eq VAR_RESULT, 0, WallyJourney_EventScript_PyreTeamUpCancel
	multi_2_vs_2 TRAINER_GRUNT_MT_PYRE_1, MtPyre_Summit_Text_Grunt1Defeat, TRAINER_GRUNT_MT_PYRE_2, MtPyre_Summit_Text_Grunt2Defeat, PARTNER_WALLY
	switch VAR_RESULT
	case 1, WallyJourney_EventScript_PyreTeamUpWon
	fadescreen FADE_TO_BLACK
	special SetCB2WhiteOut
	waitstate

WallyJourney_EventScript_PyreTeamUpWon::
	setvar VAR_WALLY_PYRE_TEAMUP, 1
	settrainerflag TRAINER_GRUNT_MT_PYRE_1
	settrainerflag TRAINER_GRUNT_MT_PYRE_2
	msgbox WallyJourney_Text_PyreWallyAfter, MSGBOX_DEFAULT
	closemessage
	@ He stays where he is to guard the way up, as he says; he's gone next time the map loads.
	setflag FLAG_HIDE_MT_PYRE_SUMMIT_WALLY
	releaseall
	end

WallyJourney_EventScript_PyreTeamUpCancel::
	special LoadPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox WallyJourney_Text_PyreWallyLater, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8009, LOCALID_MT_PYRE_SUMMIT_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_MT_PYRE_SUMMIT_WALLY
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

WallyJourney_EventScript_PyreTeamUpNotYet::
	end

WallyJourney_Text_PyreWallyArrives:
	.string "WALLY: {PLAYER}! Wait up!\p"
	.string "The LEAGUE asked me to keep an eye\n"
	.string "on MT. PYRE. TEAM AQUA is all over\l"
	.string "the summit.\p"
	.string "Two of them up ahead. We take them\n"
	.string "together. Pick your best three!$"

WallyJourney_Text_PyreWallyAfter:
	.string "WALLY: Not bad. You kept up.\p"
	.string "Go on ahead. I'll make sure nobody\n"
	.string "follows you up.$"

WallyJourney_Text_PyreWallyLater:
	.string "WALLY: Not ready? That's fine.\n"
	.string "Get your team together. I'll wait.$"

WallyJourney_Text_TitanRetreats:
	.string "The wounded giant sank back into\n"
	.string "the dark…\p"
	.string "It isn't gone. It's waiting.\n"
	.string "Rest up and come back.$"

@ Wally's Journey: postgame Fan Club collector. Hands out one famous-form Pokémon per talk,
@ in order, so special forms (Pikachu caps and costumes, Spiky-eared Pichu, Battle Bond Greninja,
@ Own Tempo Rockruff, Eternal Flower Floette) are obtainable without events.
WallyJourney_EventScript_FanClubCollector::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, WallyJourney_EventScript_FanClubCollectorNotYet
	goto_if_ge VAR_WALLY_FAN_CLUB_GIFTS, 18, WallyJourney_EventScript_FanClubCollectorDone
	msgbox WallyJourney_Text_FanClubCollectorOffer, MSGBOX_DEFAULT
	switch VAR_WALLY_FAN_CLUB_GIFTS
	case 0, WallyJourney_EventScript_FanClubGift0
	case 1, WallyJourney_EventScript_FanClubGift1
	case 2, WallyJourney_EventScript_FanClubGift2
	case 3, WallyJourney_EventScript_FanClubGift3
	case 4, WallyJourney_EventScript_FanClubGift4
	case 5, WallyJourney_EventScript_FanClubGift5
	case 6, WallyJourney_EventScript_FanClubGift6
	case 7, WallyJourney_EventScript_FanClubGift7
	case 8, WallyJourney_EventScript_FanClubGift8
	case 9, WallyJourney_EventScript_FanClubGift9
	case 10, WallyJourney_EventScript_FanClubGift10
	case 11, WallyJourney_EventScript_FanClubGift11
	case 12, WallyJourney_EventScript_FanClubGift12
	case 13, WallyJourney_EventScript_FanClubGift13
	case 14, WallyJourney_EventScript_FanClubGift14
	case 15, WallyJourney_EventScript_FanClubGift15
	case 16, WallyJourney_EventScript_FanClubGift16
	case 17, WallyJourney_EventScript_FanClubGift17
	releaseall
	end

WallyJourney_EventScript_FanClubGift0::
	givemon SPECIES_PIKACHU_ORIGINAL, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift1::
	givemon SPECIES_PIKACHU_HOENN, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift2::
	givemon SPECIES_PIKACHU_SINNOH, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift3::
	givemon SPECIES_PIKACHU_UNOVA, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift4::
	givemon SPECIES_PIKACHU_KALOS, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift5::
	givemon SPECIES_PIKACHU_ALOLA, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift6::
	givemon SPECIES_PIKACHU_PARTNER, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift7::
	givemon SPECIES_PIKACHU_WORLD, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift8::
	givemon SPECIES_PIKACHU_COSPLAY, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift9::
	givemon SPECIES_PIKACHU_ROCK_STAR, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift10::
	givemon SPECIES_PIKACHU_BELLE, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift11::
	givemon SPECIES_PIKACHU_POP_STAR, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift12::
	givemon SPECIES_PIKACHU_PHD, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift13::
	givemon SPECIES_PIKACHU_LIBRE, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift14::
	givemon SPECIES_PICHU_SPIKY_EARED, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift15::
	givemon SPECIES_GRENINJA_BATTLE_BOND, 36
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift16::
	givemon SPECIES_ROCKRUFF_OWN_TEMPO, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGift17::
	givemon SPECIES_FLOETTE_ETERNAL, 25
	goto WallyJourney_EventScript_FanClubGiftResult

WallyJourney_EventScript_FanClubGiftResult::
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Common_EventScript_NoMoreRoomForPokemon
	addvar VAR_WALLY_FAN_CLUB_GIFTS, 1
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	msgbox WallyJourney_Text_FanClubCollectorGave, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_FanClubCollectorNotYet::
	msgbox WallyJourney_Text_FanClubCollectorNotYet, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_FanClubCollectorDone::
	msgbox WallyJourney_Text_FanClubCollectorDone, MSGBOX_DEFAULT
	release
	end

WallyJourney_Text_FanClubCollectorNotYet:
	.string "I collect famous POKéMON from every\n"
	.string "region. Caps, costumes, legends…\p"
	.string "I only trade stories with CHAMPIONS,\n"
	.string "though. Come back when you've earned\l"
	.string "the title!$"
WallyJourney_Text_FanClubCollectorOffer:
	.string "The CHAMPION! I've been saving these\n"
	.string "for someone like you.\p"
	.string "They need a TRAINER who'll take\n"
	.string "them on adventures. Here's the\l"
	.string "next one in my collection!$"
WallyJourney_Text_FanClubCollectorGave:
	.string "Take good care of it!\n"
	.string "Talk to me again for the next one.$"
WallyJourney_Text_FanClubCollectorDone:
	.string "That was my whole collection!\n"
	.string "Thank you for giving them a home.$"

@ Wally's Journey: Battle Frontier training goods (money). Mints, Ability Capsule, Bottle Cap.
WallyJourney_EventScript_FrontierTrainingGoods::
	lock
	faceplayer
	msgbox WallyJourney_Text_FrontierTrainingGoods, MSGBOX_DEFAULT
	waitmessage
	pokemart WallyJourney_Pokemart_FrontierTrainingGoods
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
WallyJourney_Pokemart_FrontierTrainingGoods:
	.2byte ITEM_LONELY_MINT
	.2byte ITEM_ADAMANT_MINT
	.2byte ITEM_NAUGHTY_MINT
	.2byte ITEM_BRAVE_MINT
	.2byte ITEM_BOLD_MINT
	.2byte ITEM_IMPISH_MINT
	.2byte ITEM_LAX_MINT
	.2byte ITEM_RELAXED_MINT
	.2byte ITEM_MODEST_MINT
	.2byte ITEM_MILD_MINT
	.2byte ITEM_RASH_MINT
	.2byte ITEM_QUIET_MINT
	.2byte ITEM_CALM_MINT
	.2byte ITEM_GENTLE_MINT
	.2byte ITEM_CAREFUL_MINT
	.2byte ITEM_SASSY_MINT
	.2byte ITEM_TIMID_MINT
	.2byte ITEM_HASTY_MINT
	.2byte ITEM_JOLLY_MINT
	.2byte ITEM_NAIVE_MINT
	.2byte ITEM_SERIOUS_MINT
	.2byte ITEM_ABILITY_CAPSULE
	.2byte ITEM_BOTTLE_CAP
	pokemartlistend

@ Wally's Journey: Birch's aide rewards Pokédex research (caught count) in tiers.
WallyJourney_EventScript_DexResearchRewards::
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, IsNationalPokedexEnabled
	copyvar VAR_0x8004, VAR_RESULT
	special ScriptGetPokedexInfo
	buffernumberstring STR_VAR_2, VAR_0x8006
	msgbox WallyJourney_Text_DexResearchCount, MSGBOX_DEFAULT
WallyJourney_EventScript_DexResearchLoop::
	goto_if_ge VAR_WALLY_DEX_REWARD_TIER, 8, WallyJourney_EventScript_DexResearchAllDone
	switch VAR_WALLY_DEX_REWARD_TIER
	case 0, WallyJourney_EventScript_DexResearchTier0
	case 1, WallyJourney_EventScript_DexResearchTier1
	case 2, WallyJourney_EventScript_DexResearchTier2
	case 3, WallyJourney_EventScript_DexResearchTier3
	case 4, WallyJourney_EventScript_DexResearchTier4
	case 5, WallyJourney_EventScript_DexResearchTier5
	case 6, WallyJourney_EventScript_DexResearchTier6
	case 7, WallyJourney_EventScript_DexResearchTier7
	end

WallyJourney_EventScript_DexResearchTier0::
	goto_if_lt VAR_0x8006, 25, WallyJourney_EventScript_DexResearchNext0
	giveitem ITEM_EXP_CANDY_M, 3
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext0::
	buffernumberstring STR_VAR_1, 25
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier1::
	goto_if_lt VAR_0x8006, 50, WallyJourney_EventScript_DexResearchNext1
	giveitem ITEM_ABILITY_CAPSULE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext1::
	buffernumberstring STR_VAR_1, 50
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier2::
	goto_if_lt VAR_0x8006, 100, WallyJourney_EventScript_DexResearchNext2
	giveitem ITEM_BOTTLE_CAP, 2
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext2::
	buffernumberstring STR_VAR_1, 100
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier3::
	goto_if_lt VAR_0x8006, 200, WallyJourney_EventScript_DexResearchNext3
	giveitem ITEM_ABILITY_PATCH, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext3::
	buffernumberstring STR_VAR_1, 200
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier4::
	goto_if_lt VAR_0x8006, 300, WallyJourney_EventScript_DexResearchNext4
	giveitem ITEM_EXP_CHARM, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext4::
	buffernumberstring STR_VAR_1, 300
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier5::
	goto_if_lt VAR_0x8006, 400, WallyJourney_EventScript_DexResearchNext5
	giveitem ITEM_OVAL_CHARM, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext5::
	buffernumberstring STR_VAR_1, 400
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier6::
	goto_if_lt VAR_0x8006, 600, WallyJourney_EventScript_DexResearchNext6
	giveitem ITEM_GOLD_BOTTLE_CAP, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext6::
	buffernumberstring STR_VAR_1, 600
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchTier7::
	goto_if_lt VAR_0x8006, 850, WallyJourney_EventScript_DexResearchNext7
	giveitem ITEM_SHINY_CHARM, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	addvar VAR_WALLY_DEX_REWARD_TIER, 1
	goto WallyJourney_EventScript_DexResearchLoop
WallyJourney_EventScript_DexResearchNext7::
	buffernumberstring STR_VAR_1, 850
	msgbox WallyJourney_Text_DexResearchNextGoal, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_DexResearchAllDone::
	msgbox WallyJourney_Text_DexResearchAllDone, MSGBOX_DEFAULT
	release
	end

WallyJourney_Text_FrontierTrainingGoods:
	.string "Serious TRAINERS fine-tune their\n"
	.string "POKéMON. Mints, capsules, caps…\l"
	.string "Take a look!$"
WallyJourney_Text_DexResearchCount:
	.string "Let's see your POKéDEX data…\n"
	.string "Caught so far: {STR_VAR_2} kinds!\p"
	.string "PROF. BIRCH set aside rewards for\n"
	.string "every research milestone.$"
WallyJourney_Text_DexResearchNextGoal:
	.string "Catch {STR_VAR_1} kinds in all,\n"
	.string "then come see me again.\p"
	.string "I'll have the next reward ready!$"
WallyJourney_Text_DexResearchAllDone:
	.string "You've claimed every research\n"
	.string "reward. The PROFESSOR is thrilled!$"

@ Wally's Journey (owner direction): every Wally rival battle is mandatory and happens
@ somewhere organic, never right after a Gym fight: on the next route the story forces you
@ through (111 after badge 3, 112 after badge 4, 118 after badge 5), arriving in Lilycove
@ after badge 6, and anywhere in Mossdeep except the Gym exit after badge 7 (usually
@ stepping out of the Pokemon Center). He appears two tiles ahead of the player and walks up.
@ Losing whites out as normal; the battle happens again next time.
WallyJourney_EventScript_WallyWalksUp::
	@ Wally walks in from just off-screen along a real path (planned in C); if the player is
	@ boxed in with no path, fall back to stepping up from two tiles ahead.
	special WallyJourney_PlanWallyApproach
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_WallyWalksUpNear
	setobjectxyperm VAR_0x8009, VAR_0x8006, VAR_0x8007
	addobject VAR_0x8009
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	special WallyJourney_StartWallyApproach
	waitmovement VAR_0x8009
	special WallyJourney_FaceWally
	return

@ Same, but Wally enters from a given tile (VAR_0x8006/VAR_0x8007), e.g. a cave exit.
WallyJourney_EventScript_WallyWalksUpFrom::
	special WallyJourney_PlanWallyApproachFrom
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_WallyWalksUp
	setobjectxyperm VAR_0x8009, VAR_0x8006, VAR_0x8007
	addobject VAR_0x8009
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	special WallyJourney_StartWallyApproach
	waitmovement VAR_0x8009
	special WallyJourney_FaceWally
	return

WallyJourney_EventScript_WallyWalksUpNear::
	special StorePlayerCoordsInVars
	specialvar VAR_RESULT, GetPlayerFacingDirection
	copyvar VAR_0x8006, VAR_0x8004
	copyvar VAR_0x8007, VAR_0x8005
	call_if_eq VAR_RESULT, DIR_SOUTH, WallyJourney_EventScript_WallyAheadSouth
	call_if_eq VAR_RESULT, DIR_NORTH, WallyJourney_EventScript_WallyAheadNorth
	call_if_eq VAR_RESULT, DIR_WEST, WallyJourney_EventScript_WallyAheadWest
	call_if_eq VAR_RESULT, DIR_EAST, WallyJourney_EventScript_WallyAheadEast
	setobjectxyperm VAR_0x8009, VAR_0x8006, VAR_0x8007
	addobject VAR_0x8009
	playse SE_PIN
	call_if_eq VAR_RESULT, DIR_SOUTH, WallyJourney_EventScript_WallyStepNorth
	call_if_eq VAR_RESULT, DIR_NORTH, WallyJourney_EventScript_WallyStepSouth
	call_if_eq VAR_RESULT, DIR_WEST, WallyJourney_EventScript_WallyStepEast
	call_if_eq VAR_RESULT, DIR_EAST, WallyJourney_EventScript_WallyStepWest
	waitmovement 0
	return

WallyJourney_EventScript_WallyAheadSouth::
	addvar VAR_0x8007, 2
	return
WallyJourney_EventScript_WallyAheadNorth::
	subvar VAR_0x8007, 2
	return
WallyJourney_EventScript_WallyAheadWest::
	subvar VAR_0x8006, 2
	return
WallyJourney_EventScript_WallyAheadEast::
	addvar VAR_0x8006, 2
	return
WallyJourney_EventScript_WallyStepNorth::
	applymovement VAR_0x8009, Common_Movement_WalkUp
	return
WallyJourney_EventScript_WallyStepSouth::
	applymovement VAR_0x8009, Common_Movement_WalkDown
	return
WallyJourney_EventScript_WallyStepEast::
	applymovement VAR_0x8009, WallyJourney_Movement_StepRight
	return
WallyJourney_EventScript_WallyStepWest::
	applymovement VAR_0x8009, WallyJourney_Movement_StepLeft
	return

WallyJourney_Movement_StepRight:
	walk_right
	step_end

WallyJourney_Movement_StepLeft:
	walk_left
	step_end

WallyJourney_EventScript_WallyWalksAway::
	@ He leaves the way he came, off the screen, before he is removed.
	specialvar VAR_RESULT, WallyJourney_WallyHasPath
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_WallyWalksAwayNear
	special WallyJourney_StartWallyLeave
	waitmovement VAR_0x8009
	removeobject VAR_0x8009
	return

WallyJourney_EventScript_WallyWalksAwayNear::
	applymovement VAR_0x8009, WallyJourney_Movement_WallyLeave
	waitmovement 0
	removeobject VAR_0x8009
	return

WallyJourney_Movement_WallyLeave:
	face_away_player
	walk_fast_down
	walk_fast_down
	step_end

@ Owner request (2026-10-26): Wally's first rival battle after Wattson waits at the north end
@ of the FIERY PATH (coord triggers on the corridor below the exit) instead of firing the
@ moment the player steps onto Route 111. He walks in through the cave exit and back out.
WallyJourney_EventScript_RivalMeet3::
	lockall
	setvar VAR_0x8009, LOCALID_FIERY_PATH_RIVAL_WALLY
	clearflag FLAG_HIDE_WALLY_RIVAL_VERDANTURF
	setvar VAR_0x8006, 26
	setvar VAR_0x8007, 4
	call WallyJourney_EventScript_WallyWalksUpFrom
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalVERDANTURFIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_VERDANTURF, WallyJourney_Text_RivalVERDANTURFDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	clearflag FLAG_WALLY_NO_WHITEOUT
	call_if_ne VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalVERDANTURFWallyWon
	call_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalVERDANTURFPlayerWon
	closemessage
	setvar VAR_0x8009, LOCALID_FIERY_PATH_RIVAL_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_WALLY_RIVAL_VERDANTURF
	setvar VAR_WALLY_RIVAL_PENDING, 0
	fadedefaultbgm
	releaseall
	end

WallyJourney_EventScript_RivalMeet4::
	lockall
	setvar VAR_0x8009, LOCALID_ROUTE112_RIVAL_WALLY
	clearflag FLAG_HIDE_WALLY_RIVAL_LAVARIDGE
	call WallyJourney_EventScript_WallyWalksUp
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalLAVARIDGEIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_LAVARIDGE, WallyJourney_Text_RivalLAVARIDGEDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	clearflag FLAG_WALLY_NO_WHITEOUT
	call_if_ne VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalLAVARIDGEWallyWon
	call_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalLAVARIDGEPlayerWon
	closemessage
	setvar VAR_0x8009, LOCALID_ROUTE112_RIVAL_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_WALLY_RIVAL_LAVARIDGE
	setvar VAR_WALLY_RIVAL_PENDING, 0
	fadedefaultbgm
	releaseall
	end

WallyJourney_EventScript_RivalMeet5::
	lockall
	setvar VAR_0x8009, LOCALID_ROUTE118_RIVAL_WALLY
	clearflag FLAG_HIDE_WALLY_RIVAL_FORTREE
	call WallyJourney_EventScript_WallyWalksUp
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalFORTREEIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_FORTREE, WallyJourney_Text_RivalFORTREEDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	clearflag FLAG_WALLY_NO_WHITEOUT
	call_if_ne VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalFORTREEWallyWon
	call_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalFORTREEPlayerWon
	closemessage
	setvar VAR_0x8009, LOCALID_ROUTE118_RIVAL_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_WALLY_RIVAL_FORTREE
	setvar VAR_WALLY_RIVAL_PENDING, 0
	fadedefaultbgm
	releaseall
	end

WallyJourney_EventScript_RivalMeet6::
	lockall
	setvar VAR_0x8009, LOCALID_LILYCOVE_RIVAL_WALLY
	clearflag FLAG_HIDE_WALLY_RIVAL_LILYCOVE
	call WallyJourney_EventScript_WallyWalksUp
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalLILYCOVEIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_LILYCOVE, WallyJourney_Text_RivalLILYCOVEDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	clearflag FLAG_WALLY_NO_WHITEOUT
	call_if_ne VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalLILYCOVEWallyWon
	call_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalLILYCOVEPlayerWon
	closemessage
	setvar VAR_0x8009, LOCALID_LILYCOVE_RIVAL_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_WALLY_RIVAL_LILYCOVE
	setvar VAR_WALLY_RIVAL_PENDING, 0
	fadedefaultbgm
	releaseall
	end

WallyJourney_EventScript_RivalMeet7::
	lockall
	special StorePlayerCoordsInVars
	goto_if_ne VAR_0x8004, 38, WallyJourney_EventScript_RivalMeet7Go
	goto_if_ne VAR_0x8005, 10, WallyJourney_EventScript_RivalMeet7Go
	releaseall
	end
WallyJourney_EventScript_RivalMeet7Go::
	setvar VAR_0x8009, LOCALID_MOSSDEEP_RIVAL_WALLY
	clearflag FLAG_HIDE_WALLY_RIVAL_MOSSDEEP
	call WallyJourney_EventScript_WallyWalksUp
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox WallyJourney_Text_RivalMOSSDEEPIntro, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_WALLY_NO_WHITEOUT
	trainerbattle_no_intro TRAINER_WALLY_MOSSDEEP, WallyJourney_Text_RivalMOSSDEEPDefeat
	specialvar VAR_RESULT, GetBattleOutcome
	clearflag FLAG_WALLY_NO_WHITEOUT
	call_if_ne VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalMOSSDEEPWallyWon
	call_if_eq VAR_RESULT, B_OUTCOME_WON, WallyJourney_EventScript_RivalMOSSDEEPPlayerWon
	closemessage
	setvar VAR_0x8009, LOCALID_MOSSDEEP_RIVAL_WALLY
	call WallyJourney_EventScript_WallyWalksAway
	setflag FLAG_HIDE_WALLY_RIVAL_MOSSDEEP
	setvar VAR_WALLY_RIVAL_PENDING, 0
	fadedefaultbgm
	releaseall
	end

@ Season change notice (src/wally_journey.c, WallyJourney_TrySeasonNotice).
WallyJourney_EventScript_SeasonSpring::
	lockall
	msgbox WallyJourney_Text_SeasonSpring, MSGBOX_DEFAULT
	releaseall
	end

WallyJourney_EventScript_SeasonSummer::
	lockall
	msgbox WallyJourney_Text_SeasonSummer, MSGBOX_DEFAULT
	releaseall
	end

WallyJourney_EventScript_SeasonAutumn::
	lockall
	msgbox WallyJourney_Text_SeasonAutumn, MSGBOX_DEFAULT
	releaseall
	end

WallyJourney_EventScript_SeasonWinter::
	lockall
	msgbox WallyJourney_Text_SeasonWinter, MSGBOX_DEFAULT
	releaseall
	end

WallyJourney_Text_SeasonSpring:
	.string "Spring has arrived in HOENN.\n"
	.string "Expect warm showers on the routes.$"

WallyJourney_Text_SeasonSummer:
	.string "Summer has arrived in HOENN.\n"
	.string "Long, hot days and sudden storms.$"

WallyJourney_Text_SeasonAutumn:
	.string "Autumn has arrived in HOENN.\n"
	.string "The air has turned cool and grey.$"

WallyJourney_Text_SeasonWinter:
	.string "Winter has arrived in HOENN.\n"
	.string "Snow is falling in the mountains.$"


@ Wally's Journey: Hyper Training (Lilycove Pokemon Center). Bottle Cap = one stat, Gold Bottle Cap = all six.
@ Stat ids: 0 HP, 1 Atk, 2 Def, 3 Speed, 4 SpAtk, 5 SpDef (enum Stat).
@ Pokemon must be level 50+, matching Scarlet/Violet.
WallyJourney_EventScript_HyperTrainer::
	lock
	faceplayer
	msgbox WallyJourney_Text_HyperTrainerIntro, MSGBOX_DEFAULT
	checkitem ITEM_GOLD_BOTTLE_CAP
	copyvar VAR_0x8009, VAR_RESULT
	checkitem ITEM_BOTTLE_CAP
	copyvar VAR_0x800A, VAR_RESULT
	goto_if_eq VAR_0x8009, TRUE, WallyJourney_EventScript_HyperTrainerAsk
	goto_if_eq VAR_0x800A, TRUE, WallyJourney_EventScript_HyperTrainerAsk
	msgbox WallyJourney_Text_HyperTrainerNoCaps, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_HyperTrainerAsk::
	msgbox WallyJourney_Text_HyperTrainerWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, WallyJourney_EventScript_HyperTrainerCancel
	specialvar VAR_RESULT, WallyJourney_GetPartyMonLevel
	goto_if_eq VAR_RESULT, 0, WallyJourney_EventScript_HyperTrainerCancel
	goto_if_lt VAR_RESULT, 50, WallyJourney_EventScript_HyperTrainerTooLow
	bufferpartymonnick STR_VAR_1, VAR_0x8004
	goto_if_ne VAR_0x8009, TRUE, WallyJourney_EventScript_HyperTrainerPickStat
	msgbox WallyJourney_Text_HyperTrainerUseGold, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WallyJourney_EventScript_HyperTrainerGold
	goto_if_ne VAR_0x800A, TRUE, WallyJourney_EventScript_HyperTrainerCancel

WallyJourney_EventScript_HyperTrainerPickStat::
	msgbox WallyJourney_Text_HyperTrainerWhichStat, MSGBOX_DEFAULT
	dynmultichoice 0, 0, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, WallyJourney_Text_StatHP, WallyJourney_Text_StatAtk, WallyJourney_Text_StatDef, WallyJourney_Text_StatSpAtk, WallyJourney_Text_StatSpDef, WallyJourney_Text_StatSpeed
	switch VAR_RESULT
	case 0, WallyJourney_EventScript_HyperTrainHP
	case 1, WallyJourney_EventScript_HyperTrainAtk
	case 2, WallyJourney_EventScript_HyperTrainDef
	case 3, WallyJourney_EventScript_HyperTrainSpAtk
	case 4, WallyJourney_EventScript_HyperTrainSpDef
	case 5, WallyJourney_EventScript_HyperTrainSpeed
	goto WallyJourney_EventScript_HyperTrainerCancel

WallyJourney_EventScript_HyperTrainHP::
	canhypertrain 0, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 0, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap
WallyJourney_EventScript_HyperTrainAtk::
	canhypertrain 1, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 1, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap
WallyJourney_EventScript_HyperTrainDef::
	canhypertrain 2, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 2, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap
WallyJourney_EventScript_HyperTrainSpAtk::
	canhypertrain 4, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 4, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap
WallyJourney_EventScript_HyperTrainSpDef::
	canhypertrain 5, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 5, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap
WallyJourney_EventScript_HyperTrainSpeed::
	canhypertrain 3, VAR_0x8004
	goto_if_eq VAR_RESULT, FALSE, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 3, VAR_0x8004
	goto WallyJourney_EventScript_HyperTrainerDoneCap

WallyJourney_EventScript_HyperTrainerDoneCap::
	removeitem ITEM_BOTTLE_CAP
	goto WallyJourney_EventScript_HyperTrainerDone

WallyJourney_EventScript_HyperTrainerGold::
	setvar VAR_0x8005, 0
	canhypertrain 0, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	canhypertrain 1, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	canhypertrain 2, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	canhypertrain 4, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	canhypertrain 5, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	canhypertrain 3, VAR_0x8004
	call_if_eq VAR_RESULT, TRUE, WallyJourney_EventScript_HyperTrainerCount
	goto_if_eq VAR_0x8005, 0, WallyJourney_EventScript_HyperTrainerMaxed
	hypertrain 0, VAR_0x8004
	hypertrain 1, VAR_0x8004
	hypertrain 2, VAR_0x8004
	hypertrain 4, VAR_0x8004
	hypertrain 5, VAR_0x8004
	hypertrain 3, VAR_0x8004
	removeitem ITEM_GOLD_BOTTLE_CAP
	goto WallyJourney_EventScript_HyperTrainerDone

WallyJourney_EventScript_HyperTrainerCount::
	addvar VAR_0x8005, 1
	return

WallyJourney_EventScript_HyperTrainerDone::
	playfanfare MUS_LEVEL_UP
	msgbox WallyJourney_Text_HyperTrainerDone, MSGBOX_DEFAULT
	waitfanfare
	release
	end

WallyJourney_EventScript_HyperTrainerMaxed::
	msgbox WallyJourney_Text_HyperTrainerMaxed, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_HyperTrainerTooLow::
	msgbox WallyJourney_Text_HyperTrainerTooLow, MSGBOX_DEFAULT
	release
	end

WallyJourney_EventScript_HyperTrainerCancel::
	msgbox WallyJourney_Text_HyperTrainerCancel, MSGBOX_DEFAULT
	release
	end

WallyJourney_Text_HyperTrainerIntro:
	.string "I train POKéMON past their limits.\n"
	.string "HYPER TRAINING, they call it.\p"
	.string "Bring me a BOTTLE CAP and I'll max\n"
	.string "out one stat. A GOLD BOTTLE CAP?\l"
	.string "All six at once.$"

WallyJourney_Text_HyperTrainerNoCaps:
	.string "No BOTTLE CAPS, though.\p"
	.string "They're rare. Folks say a few are\n"
	.string "hidden in the deepest places in\l"
	.string "HOENN. Keep your eyes open.$"

WallyJourney_Text_HyperTrainerWhichMon:
	.string "Which POKéMON? It has to be at\n"
	.string "least level 50 to handle it.$"

WallyJourney_Text_HyperTrainerUseGold:
	.string "Use your GOLD BOTTLE CAP to max out\n"
	.string "every stat of {STR_VAR_1}?$"

WallyJourney_Text_HyperTrainerWhichStat:
	.string "Which stat should {STR_VAR_1} push\n"
	.string "to its limit?$"

WallyJourney_Text_HyperTrainerDone:
	.string "There! {STR_VAR_1} has been pushed\n"
	.string "to its absolute limit!$"

WallyJourney_Text_HyperTrainerMaxed:
	.string "That's already as high as it goes.\n"
	.string "Keep your cap for another one.$"

WallyJourney_Text_HyperTrainerTooLow:
	.string "Too young for this. Bring it back\n"
	.string "at level 50 or higher.$"

WallyJourney_Text_HyperTrainerCancel:
	.string "Come back when you're ready.$"

WallyJourney_Text_StatHP:
	.string "HP$"
WallyJourney_Text_StatAtk:
	.string "ATTACK$"
WallyJourney_Text_StatDef:
	.string "DEFENSE$"
WallyJourney_Text_StatSpAtk:
	.string "SP. ATK$"
WallyJourney_Text_StatSpDef:
	.string "SP. DEF$"
WallyJourney_Text_StatSpeed:
	.string "SPEED$"

@ Wally's Journey: from Route 111 on, Wally is far stronger. Losing doesn't white out;
@ the story continues either way with its own lines.
WallyJourney_EventScript_RivalVERDANTURFWallyWon::
	special HealPlayerParty
	msgbox WallyJourney_Text_RivalVERDANTURFWallyWon, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_RivalVERDANTURFPlayerWon::
	msgbox WallyJourney_Text_RivalVERDANTURFPost, MSGBOX_DEFAULT
	return

WallyJourney_Text_RivalVERDANTURFWallyWon:
	.string "WALLY: I... I won? I actually won!\p"
	.string "I'm sorry, {PLAYER}! I mean...\n"
	.string "thank you! I've never felt like\l"
	.string "this before!$"

WallyJourney_EventScript_RivalLAVARIDGEWallyWon::
	special HealPlayerParty
	msgbox WallyJourney_Text_RivalLAVARIDGEWallyWon, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_RivalLAVARIDGEPlayerWon::
	msgbox WallyJourney_Text_RivalLAVARIDGEPost, MSGBOX_DEFAULT
	return

WallyJourney_Text_RivalLAVARIDGEWallyWon:
	.string "WALLY: I won again! My team really\n"
	.string "is getting strong.\p"
	.string "Maybe I'm not the one who needs\n"
	.string "to catch up anymore.$"

WallyJourney_EventScript_RivalFORTREEWallyWon::
	special HealPlayerParty
	msgbox WallyJourney_Text_RivalFORTREEWallyWon, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_RivalFORTREEPlayerWon::
	msgbox WallyJourney_Text_RivalFORTREEPost, MSGBOX_DEFAULT
	return

WallyJourney_Text_RivalFORTREEWallyWon:
	.string "WALLY: See? I told you.\p"
	.string "You should take me seriously\n"
	.string "from now on.$"

WallyJourney_EventScript_RivalLILYCOVEWallyWon::
	special HealPlayerParty
	msgbox WallyJourney_Text_RivalLILYCOVEWallyWon, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_RivalLILYCOVEPlayerWon::
	msgbox WallyJourney_Text_RivalLILYCOVEPost, MSGBOX_DEFAULT
	return

WallyJourney_Text_RivalLILYCOVEWallyWon:
	.string "WALLY: Every move had a purpose.\n"
	.string "Yours didn't.\p"
	.string "I'll see you at the LEAGUE.\n"
	.string "If you make it.$"

WallyJourney_EventScript_RivalMOSSDEEPWallyWon::
	special HealPlayerParty
	msgbox WallyJourney_Text_RivalMOSSDEEPWallyWon, MSGBOX_DEFAULT
	return

WallyJourney_EventScript_RivalMOSSDEEPPlayerWon::
	msgbox WallyJourney_Text_RivalMOSSDEEPPost, MSGBOX_DEFAULT
	return

WallyJourney_Text_RivalMOSSDEEPWallyWon:
	.string "WALLY: That's what I thought.\p"
	.string "Don't worry. I'll still help with\n"
	.string "the weather. Then I'm off to\l"
	.string "the LEAGUE.$"

