MtPyre_5F_MapScripts::
	.byte 0

@ Seems like the scripts for the 4F and 5F were swapped

MtPyre_4F_EventScript_Tasha::
	trainerbattle_single TRAINER_TASHA, MtPyre_4F_Text_TashaIntro, MtPyre_4F_Text_TashaDefeat
	msgbox MtPyre_4F_Text_TashaPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_4F_Text_TashaIntro:
	.string "I love all things horrifying…\n"
	.string "When I'm here… I shiver…$"

MtPyre_4F_Text_TashaDefeat:
	.string "Losing, I dislike…$"

MtPyre_4F_Text_TashaPostBattle:
	.string "There are so many new stones up\n"
	.string "here every year…\p"
	.string "People from SLATEPORT, mostly.\n"
	.string "POKéMON from the reef.$"

