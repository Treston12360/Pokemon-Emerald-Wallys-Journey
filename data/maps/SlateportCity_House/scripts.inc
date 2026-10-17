SlateportCity_House_MapScripts::
	.byte 0

SlateportCity_House_EventScript_PokefanM::
	msgbox SlateportCity_House_Text_NatureToDoWithStatGains, MSGBOX_NPC
	end

SlateportCity_House_EventScript_Girl::
	msgbox SlateportCity_House_Text_MustBeGoingToBattleTent, MSGBOX_NPC
	end

SlateportCity_House_Text_NatureToDoWithStatGains:
	.string "My POKéMON has a HASTY nature.\p"
	.string "It's faster than my others. Natures\n"
	.string "shape how they grow. Same with\l"
	.string "people, my mom says.$"

SlateportCity_House_Text_MustBeGoingToBattleTent:
	.string "You're a TRAINER, right?\p"
	.string "Then you're here for the BATTLE\n"
	.string "TENT. Or for CAPT. STERN's\l"
	.string "autograph.$"

