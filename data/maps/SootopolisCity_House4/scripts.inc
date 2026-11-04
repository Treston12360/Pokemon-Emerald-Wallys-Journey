SootopolisCity_House4_MapScripts::
	.byte 0

SootopolisCity_House4_EventScript_Man::
	msgbox SootopolisCity_House4_Text_AncientTreasuresWaitingInSea, MSGBOX_NPC
	end

SootopolisCity_House4_EventScript_Woman::
	msgbox SootopolisCity_House4_Text_StrollUnderwaterWithPokemon, MSGBOX_NPC
	end

SootopolisCity_House4_EventScript_Azumarill::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_AZUMARILL, CRY_MODE_NORMAL
	msgbox SootopolisCity_House4_Text_Azumarill, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SootopolisCity_House4_Text_AncientTreasuresWaitingInSea:
	.string "Listen up. There's supposed to be an\n"
	.string "ancient ruin in the sea near here.\p"
	.string "Divers go looking every year.\n"
	.string "Nobody finds it.\p"
	.string "Maybe it doesn't want to be found.$"

SootopolisCity_House4_Text_StrollUnderwaterWithPokemon:
	.string "Juan taught Wallace everything.\n"
	.string "Dance, water, patience.\p"
	.string "When Wallace left for the League,\n"
	.string "Juan came out of retirement to keep\l"
	.string "the Gym.\p"
	.string "He says teaching is how you stay\n"
	.string "young. His back disagrees.$"

SootopolisCity_House4_Text_Azumarill:
	.string "Azumarill: Marurii.$"

