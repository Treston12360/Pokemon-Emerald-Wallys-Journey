RustboroCity_CuttersHouse_MapScripts::
	.byte 0

RustboroCity_CuttersHouse_EventScript_Cutter::
	lock
	faceplayer
	msgbox RustboroCity_CuttersHouse_Text_NewCutDialogue, MSGBOX_DEFAULT
	release
	end

RustboroCity_CuttersHouse_EventScript_ExplainCut::
	msgbox RustboroCity_CuttersHouse_Text_ExplainCut, MSGBOX_DEFAULT
	release
	end

RustboroCity_CuttersHouse_EventScript_Lass::
	msgbox RustboroCity_CuttersHouse_Text_DadHelpedClearLandOfTrees, MSGBOX_NPC
	end

RustboroCity_CuttersHouse_Text_YouCanPutThisHMToGoodUse:
	.string "That determined expression…\n"
	.string "That limber way you move…\l"
	.string "And your well-trained POKéMON…\p"
	.string "You're obviously a skilled TRAINER!\p"
	.string "No, wait, don't say a word.\n"
	.string "I can tell just by looking at you.\p"
	.string "I'm sure that you can put this\n"
	.string "HIDDEN MACHINE to good use.\p"
	.string "No need to be modest or shy.\n"
	.string "Go on, take it!$"

RustboroCity_CuttersHouse_Text_ExplainCut:
.string "CUT can clear thin trees outside\n"
	.string "battle.\p"
	.string "Once you've earned the STONE\n"
	.string "BADGE and have HM01, any of your\l"
	.string "POKéMON that could learn CUT can\l"
	.string "clear the way.\p"
	.string "You don't need to teach it CUT.\n"
	.string "The HM is your field license.$"

RustboroCity_CuttersHouse_Text_DadHelpedClearLandOfTrees:
	.string "When they were expanding the city of\n"
	.string "RUSTBORO, my dad helped out.\p"
	.string "He made his POKéMON use CUT to clear\n"
	.string "the land of trees.$"


RustboroCity_CuttersHouse_Text_NewCutDialogue:
.string "Those little trees grow fast!\p"
	.string "With the STONE BADGE and HM01,\n"
	.string "a compatible POKéMON can clear\l"
	.string "them without learning CUT.\p"
	.string "Good luck out there!$"
