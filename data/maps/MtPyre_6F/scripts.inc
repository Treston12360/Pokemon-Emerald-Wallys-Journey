MtPyre_6F_MapScripts::
	.byte 0

MtPyre_6F_EventScript_Valerie::
	trainerbattle_single TRAINER_VALERIE_1, MtPyre_6F_Text_ValerieIntro, MtPyre_6F_Text_ValerieDefeat, MtPyre_6F_EventScript_RegisterValerie
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MtPyre_6F_EventScript_RematchValerie
	msgbox MtPyre_6F_Text_ValeriePostBattle, MSGBOX_DEFAULT
	release
	end

MtPyre_6F_EventScript_RegisterValerie::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	msgbox MtPyre_6F_Text_ValerieRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_VALERIE_1
	release
	end

MtPyre_6F_EventScript_RematchValerie::
	trainerbattle_rematch TRAINER_VALERIE_1, MtPyre_6F_Text_ValerieRematchIntro, MtPyre_6F_Text_ValerieRematchDefeat
	msgbox MtPyre_6F_Text_ValeriePostRematch, MSGBOX_AUTOCLOSE
	end

MtPyre_6F_EventScript_Cedric::
	trainerbattle_single TRAINER_CEDRIC, MtPyre_6F_Text_CedricIntro, MtPyre_6F_Text_CedricDefeat
	msgbox MtPyre_6F_Text_CedricPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_6F_Text_ValerieIntro:
	.string "Up here… I feel the spirits\n"
	.string "moving through me…$"

MtPyre_6F_Text_ValerieDefeat:
	.string "The spirits… let go\n"
	.string "of me…$"

MtPyre_6F_Text_ValeriePostBattle:
	.string "The old keepers say the ORBS on\n"
	.string "the summit sing two old songs.\p"
	.string "One calms the land. One calms the\n"
	.string "sea. Neither commands anything.$"

MtPyre_6F_Text_ValerieRegister:
	.string "Fufufu… I lost the match, but…\n"
	.string "I have this little ability…\p"
	.string "Without ever laying my hands on\n"
	.string "your POKéNAV… Hiyah!$"

MtPyre_6F_Text_ValerieRematchIntro:
	.string "Something's behind you…\n"
	.string "It wants to watch…$"

MtPyre_6F_Text_ValerieRematchDefeat:
	.string "It faded away…\n"
	.string "It liked you.$"

MtPyre_6F_Text_ValeriePostRematch:
	.string "The POKéMON at rest here…\n"
	.string "Sometimes they play with visitors.$"

MtPyre_6F_Text_CedricIntro:
	.string "Lost? This mountain swallows\n"
	.string "people! I'll guide you!$"

MtPyre_6F_Text_CedricDefeat:
	.string "…You weren't lost at all,\n"
	.string "were you?$"

MtPyre_6F_Text_CedricPostBattle:
	.string "I thought a lost TRAINER\n"
	.string "would be easy to defeat.\p"
	.string "But I saw how frightened\n"
	.string "people look up here.\p"
	.string "Next time I'll offer to\n"
	.string "show them the way instead.$"

