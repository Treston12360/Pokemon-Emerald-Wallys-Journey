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
	.string "And your well-trained Pokémon…\p"
	.string "You're obviously a skilled Trainer!\p"
	.string "No, wait, don't say a word.\n"
	.string "I can tell just by looking at you.\p"
	.string "I'm sure that you can put this\n"
	.string "Hidden Machine to good use.\p"
	.string "No need to be modest or shy.\n"
	.string "Go on, take it!$"

RustboroCity_CuttersHouse_Text_ExplainCut:
	.string "Cut clears small trees when you\n"
	.string "have the Stone Badge and HM01.\p"
	.string "Just press A facing a tree.\n"
	.string "You don't need a Pokémon that\l"
	.string "knows Cut or can learn it.$"

RustboroCity_CuttersHouse_Text_DadHelpedClearLandOfTrees:
	.string "My dad helped clear the forest when\n"
	.string "Rustboro grew.\p"
	.string "He was proud of it back then.\p"
	.string "Now when it rains, the hillside\n"
	.string "washes into the creek. He doesn't\l"
	.string "talk about it anymore.$"


RustboroCity_CuttersHouse_Text_NewCutDialogue:
	.string "Those little trees grow fast!\p"
	.string "Once you've earned the Stone\n"
	.string "Badge and received HM01,\l"
	.string "face a tree and press A.\p"
	.string "You won't need to teach Cut\n"
	.string "to any of your Pokémon.$"
