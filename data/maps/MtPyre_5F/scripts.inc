MtPyre_5F_MapScripts::
	.byte 0

@ Seems like the scripts for the 4F and 5F were swapped

MtPyre_4F_EventScript_Tasha::
	trainerbattle_single TRAINER_TASHA, MtPyre_4F_Text_TashaIntro, MtPyre_4F_Text_TashaDefeat
	msgbox MtPyre_4F_Text_TashaPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_4F_Text_TashaIntro:
	.string "I love horrifying things…\p"
	.string "PHOEBE grew up on this mountain. She\n"
	.string "said it's lonely, not scary. Liar…$"

MtPyre_4F_Text_TashaDefeat:
	.string "Losing… is the one horror\n"
	.string "I don't like.$"

MtPyre_4F_Text_TashaPostBattle:
	.string "There are so many new stones up\n"
	.string "here every year…\p"
	.string "People from SLATEPORT, mostly.\n"
	.string "POKéMON from the reef.$"

