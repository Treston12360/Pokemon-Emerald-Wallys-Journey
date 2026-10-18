SlateportCity_House_MapScripts::
	.byte 0

SlateportCity_House_EventScript_PokefanM::
	msgbox SlateportCity_House_Text_NatureToDoWithStatGains, MSGBOX_NPC
	end

SlateportCity_House_EventScript_Girl::
	msgbox SlateportCity_House_Text_MustBeGoingToBattleTent, MSGBOX_NPC
	end

SlateportCity_House_Text_NatureToDoWithStatGains:
	.string "My grandmother had a story about a\n"
	.string "titan of the sea.\p"
	.string "When people took too much from the\n"
	.string "ocean, it rose and swallowed the\l"
	.string "coast.\p"
	.string "She told it to make us behave.\n"
	.string "Lately it doesn't feel like a story.$"

SlateportCity_House_Text_MustBeGoingToBattleTent:
	.string "You're a TRAINER, right?\p"
	.string "Then you're here for the BATTLE\n"
	.string "TENT. Or for CAPT. STERN's\l"
	.string "autograph.$"

