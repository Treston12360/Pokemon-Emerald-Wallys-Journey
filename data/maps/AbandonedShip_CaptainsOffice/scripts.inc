AbandonedShip_CaptainsOffice_MapScripts::
	.byte 0

AbandonedShip_CaptainsOffice_EventScript_CaptSternAide::
	lock
	faceplayer
	goto_if_set FLAG_EXCHANGED_SCANNER, AbandonedShip_CaptainsOffice_EventScript_ThisIsSSCactus
	checkitem ITEM_SCANNER
	goto_if_eq VAR_RESULT, TRUE, AbandonedShip_CaptainsOffice_EventScript_CanYouDeliverScanner
	goto_if_set FLAG_ITEM_ABANDONED_SHIP_HIDDEN_FLOOR_ROOM_2_SCANNER, AbandonedShip_CaptainsOffice_EventScript_ThisIsSSCactus
	msgbox AbandonedShip_CaptainsOffice_Text_NoSuccessFindingScanner, MSGBOX_DEFAULT
	release
	end

AbandonedShip_CaptainsOffice_EventScript_CanYouDeliverScanner::
	msgbox AbandonedShip_CaptainsOffice_Text_OhCanYouDeliverScanner, MSGBOX_DEFAULT
	release
	end

AbandonedShip_CaptainsOffice_EventScript_ThisIsSSCactus::
	msgbox AbandonedShip_CaptainsOffice_Text_ThisIsSSCactus, MSGBOX_DEFAULT
	release
	end

AbandonedShip_CaptainsOffice_Text_NoSuccessFindingScanner:
	.string "I'm investigating this ship on behalf\n"
	.string "of Capt. Stern.\p"
	.string "He also asked me to find a Scanner,\n"
	.string "but I haven't had any success…$"

AbandonedShip_CaptainsOffice_Text_OhCanYouDeliverScanner:
	.string "Oh! That's a Scanner!\p"
	.string "Listen, can I get you to deliver that\n"
	.string "to Capt. Stern?\p"
	.string "I want to investigate this ship a\n"
	.string "little more.$"

AbandonedShip_CaptainsOffice_Text_ThisIsSSCactus:
	.string "This ship is the S.S. Cactus.\p"
	.string "She went down in the great storm,\n"
	.string "forty years ago. Most of the crew\l"
	.string "got off.\p"
	.string "One deckhand kept carrying people to\n"
	.string "the lifeboats. Folks say he's on the\l"
	.string "Elite Four now.$"

