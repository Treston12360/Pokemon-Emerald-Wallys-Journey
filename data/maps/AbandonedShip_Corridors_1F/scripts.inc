AbandonedShip_Corridors_1F_MapScripts::
	.byte 0

AbandonedShip_Corridors_1F_EventScript_Youngster::
	msgbox AbandonedShip_Corridors_1F_Text_IsntItFunHere, MSGBOX_NPC
	end

AbandonedShip_Corridors_1F_EventScript_Charlie::
	trainerbattle_single TRAINER_CHARLIE, AbandonedShip_Corridors_1F_Text_CharlieIntro, AbandonedShip_Corridors_1F_Text_CharlieDefeat
	msgbox AbandonedShip_Corridors_1F_Text_CharliePostBattle, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Corridors_1F_Text_CharlieIntro:
	.string "What's so funny about bringing my\n"
	.string "inner tube onto a shipwreck?$"

AbandonedShip_Corridors_1F_Text_CharlieDefeat:
	.string "Whoa! Overwhelmed!$"

AbandonedShip_Corridors_1F_Text_CharliePostBattle:
	.string "Throwing POKé BALLS while holding\n"
	.string "an inner tube is hard!$"

AbandonedShip_Corridors_1F_Text_IsntItFunHere:
	.string "Isn't it fun here?\p"
	.string "My grandpa won't come near it. He\n"
	.string "says the sea took enough people that\l"
	.string "night. It doesn't need us poking\l"
	.string "around.$"
