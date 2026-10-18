RustboroCity_Flat2_3F_MapScripts::
	.byte 0

RustboroCity_Flat2_3F_EventScript_DevonEmployee::
	msgbox RustboroCity_Flat2_3F_Text_PresidentCollectsRareStones, MSGBOX_NPC
	end

RustboroCity_Flat2_3F_EventScript_Woman::
	msgbox RustboroCity_Flat2_3F_Text_PresidentsSonAlsoCollectsRareStones, MSGBOX_NPC
	end

RustboroCity_Flat2_3F_Text_PresidentCollectsRareStones:
	.string "MR. STONE collects rare stones.\p"
	.string "People say he started so he'd have\n"
	.string "something to talk about with his son.$"

RustboroCity_Flat2_3F_Text_PresidentsSonAlsoCollectsRareStones:
	.string "STEVEN was supposed to run DEVON\n"
	.string "someday.\p"
	.string "Instead he spent his teens in\n"
	.string "GRANITE CAVE with a lantern.\p"
	.string "He told his father, “Stones don't\n"
	.string "pretend to be something else.”\l"
	.string "They didn't talk for a year.$"

