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
