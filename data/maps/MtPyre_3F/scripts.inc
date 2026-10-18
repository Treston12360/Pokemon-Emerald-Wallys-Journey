MtPyre_3F_MapScripts::
	.byte 0

MtPyre_3F_EventScript_William::
	trainerbattle_single TRAINER_WILLIAM, MtPyre_3F_Text_WilliamIntro, MtPyre_3F_Text_WilliamDefeat
	msgbox MtPyre_3F_Text_WilliamPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_3F_EventScript_Kayla::
	trainerbattle_single TRAINER_KAYLA, MtPyre_3F_Text_KaylaIntro, MtPyre_3F_Text_KaylaDefeat
	msgbox MtPyre_3F_Text_KaylaPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_3F_EventScript_Gabrielle::
	trainerbattle_single TRAINER_GABRIELLE_1, MtPyre_3F_Text_GabrielleIntro, MtPyre_3F_Text_GabrielleDefeat, MtPyre_3F_EventScript_RegisterGabrielle
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MtPyre_3F_EventScript_RematchGabrielle
	msgbox MtPyre_3F_Text_GabriellePostBattle, MSGBOX_DEFAULT
	release
	end

MtPyre_3F_EventScript_RegisterGabrielle::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	msgbox MtPyre_3F_Text_GabrielleRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_GABRIELLE_1
	release
	end

MtPyre_3F_EventScript_RematchGabrielle::
	trainerbattle_rematch TRAINER_GABRIELLE_1, MtPyre_3F_Text_GabrielleRematchIntro, MtPyre_3F_Text_GabrielleRematchDefeat
	msgbox MtPyre_3F_Text_GabriellePostRematch, MSGBOX_AUTOCLOSE
	end

MtPyre_3F_Text_WilliamIntro:
	.string "This mountain sharpens my mind!\p"
	.string "The old texts say the ORB “soothes”\n"
	.string "the titan. Some read “commands.” Fools.$"

MtPyre_3F_Text_WilliamDefeat:
	.string "I drown in self-pity…\n"
	.string "in a graveyard, no less.$"

MtPyre_3F_Text_WilliamPostBattle:
	.string "I thought this mountain would\n"
	.string "make my powers greater.\p"
	.string "But all I've done is talk\n"
	.string "about myself.\p"
	.string "Perhaps listening matters\n"
	.string "more than being heard.$"

MtPyre_3F_Text_KaylaIntro:
	.string "A child, up here?\p"
	.string "PHOEBE climbed these stairs at your\n"
	.string "age. You're no PHOEBE. Battle!$"

MtPyre_3F_Text_KaylaDefeat:
	.string "I lost… cleanly. Ugh.$"

MtPyre_3F_Text_KaylaPostBattle:
	.string "My training still isn't enough…\n"
	.string "On to the summit! Go, me!$"

MtPyre_3F_Text_GabrielleIntro:
	.string "Why have you come? To remember\n"
	.string "someone?\p"
	.string "So many new stones since the reefs\n"
	.string "began dying. So many.$"

MtPyre_3F_Text_GabrielleDefeat:
	.string "Amazing. The spirits\n"
	.string "watched you closely.$"

MtPyre_3F_Text_GabriellePostBattle:
	.string "The partners we've lost taught\n"
	.string "us things no victory could.\p"
	.string "The ones beside us still\n"
	.string "have more to teach.\p"
	.string "And the ones we'll meet?\n"
	.string "They deserve an open heart.$"

MtPyre_3F_Text_GabrielleRegister:
	.string "I would like to see your POKéMON\n"
	.string "when they grow up some more…\p"
	.string "Please, I need to see your POKéNAV.$"

MtPyre_3F_Text_GabrielleRematchIntro:
	.string "Oh, it's you… Come to show me how\n"
	.string "your POKéMON have grown?$"

MtPyre_3F_Text_GabrielleRematchDefeat:
	.string "You've grown too.\n"
	.string "They can see it.$"

MtPyre_3F_Text_GabriellePostRematch:
	.string "The POKéMON who've passed. The\n"
	.string "POKéMON with you. The ones to come.\p"
	.string "Cherish them all the same.\n"
	.string "You already do.$"

