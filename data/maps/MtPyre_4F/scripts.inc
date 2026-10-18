MtPyre_4F_MapScripts::
	.byte 0

@ Seems like the scripts for the 4F and 5F were swapped

MtPyre_5F_EventScript_Atsushi::
	trainerbattle_single TRAINER_ATSUSHI, MtPyre_5F_Text_AtsushiIntro, MtPyre_5F_Text_AtsushiDefeat
	msgbox MtPyre_5F_Text_AtsushiPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_5F_Text_AtsushiIntro:
	.string "Teacher, you rest here now.\p"
	.string "You taught me the land and sea must\n"
	.string "never be woken. Watch me grow!$"

MtPyre_5F_Text_AtsushiDefeat:
	.string "Teacher…\n"
	.string "I'm sorry.$"

MtPyre_5F_Text_AtsushiPostBattle:
	.string "My teacher can't rest until I'm\n"
	.string "strong. I'll keep training.$"

