MeteorFalls_1F_2R_MapScripts::
	.byte 0

MeteorFalls_1F_2R_EventScript_Nicolas::
	trainerbattle_single TRAINER_NICOLAS_1, MeteorFalls_1F_2R_Text_NicolasIntro, MeteorFalls_1F_2R_Text_NicolasDefeat, MeteorFalls_1F_2R_EventScript_RegisterNicolas
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MeteorFalls_1F_2R_EventScript_RematchNicolas
	msgbox MeteorFalls_1F_2R_Text_NicolasPostBattle, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_2R_EventScript_RegisterNicolas::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	msgbox MeteorFalls_1F_2R_Text_NicolasRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_NICOLAS_1
	release
	end

MeteorFalls_1F_2R_EventScript_RematchNicolas::
	trainerbattle_rematch TRAINER_NICOLAS_1, MeteorFalls_1F_2R_Text_NicolasRematchIntro, MeteorFalls_1F_2R_Text_NicolasRematchDefeat
	msgbox MeteorFalls_1F_2R_Text_NicolasPostRematch, MSGBOX_AUTOCLOSE
	end

MeteorFalls_1F_2R_EventScript_John::
	trainerbattle_double TRAINER_JOHN_AND_JAY_1, MeteorFalls_1F_2R_Text_JohnIntro, MeteorFalls_1F_2R_Text_JohnDefeat, MeteorFalls_1F_2R_Text_JohnNotEnoughMons, MeteorFalls_1F_2R_EventScript_RegisterJohn
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MeteorFalls_1F_2R_EventScript_RematchJohn
	msgbox MeteorFalls_1F_2R_Text_JohnPostBattle, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_2R_EventScript_RegisterJohn::
	msgbox MeteorFalls_1F_2R_Text_JohnRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_JOHN_AND_JAY_1
	release
	end

MeteorFalls_1F_2R_EventScript_RematchJohn::
	trainerbattle_rematch_double TRAINER_JOHN_AND_JAY_1, MeteorFalls_1F_2R_Text_JohnRematchIntro, MeteorFalls_1F_2R_Text_JohnRematchDefeat, MeteorFalls_1F_2R_Text_JohnRematchNotEnoughMons
	msgbox MeteorFalls_1F_2R_Text_JohnPostRematch, MSGBOX_AUTOCLOSE
	end

MeteorFalls_1F_2R_EventScript_Jay::
	trainerbattle_double TRAINER_JOHN_AND_JAY_1, MeteorFalls_1F_2R_Text_JayIntro, MeteorFalls_1F_2R_Text_JayDefeat, MeteorFalls_1F_2R_Text_JayNotEnoughMons, MeteorFalls_1F_2R_EventScript_RegisterJay
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, MeteorFalls_1F_2R_EventScript_RematchJay
	msgbox MeteorFalls_1F_2R_Text_JayPostBattle, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_2R_EventScript_RegisterJay::
	msgbox MeteorFalls_1F_2R_Text_JohnRegister, MSGBOX_DEFAULT  @ John speaks for both during register
	register_matchcall TRAINER_JOHN_AND_JAY_1
	release
	end

MeteorFalls_1F_2R_EventScript_RematchJay::
	trainerbattle_rematch_double TRAINER_JOHN_AND_JAY_1, MeteorFalls_1F_2R_Text_JayRematchIntro, MeteorFalls_1F_2R_Text_JayRematchDefeat, MeteorFalls_1F_2R_Text_JayRematchNotEnoughMons
	msgbox MeteorFalls_1F_2R_Text_JayPostRematch, MSGBOX_AUTOCLOSE
	end

MeteorFalls_1F_2R_Text_NicolasIntro:
	.string "DRAGON users train here, where the\n"
	.string "stars fell.\p"
	.string "DRAGON clans have trained under\n"
	.string "these falls for centuries.$"

MeteorFalls_1F_2R_Text_NicolasDefeat:
	.string "Urgh!\n"
	.string "I didn't expect such strength!$"

MeteorFalls_1F_2R_Text_NicolasPostBattle:
	.string "My clan's elders say the falls were\n"
	.string "made by a dragon that ate stars.\p"
	.string "I've never seen it. But I train like\n"
	.string "it's watching.$"

MeteorFalls_1F_2R_Text_NicolasRegister:
	.string "I want to know more about your power.\n"
	.string "Let me register you in my POKéNAV.$"

MeteorFalls_1F_2R_Text_NicolasRematchIntro:
	.string "I've trained under the falls every\n"
	.string "day since we met. Battle!$"

MeteorFalls_1F_2R_Text_NicolasRematchDefeat:
	.string "Urgh!\n"
	.string "You're stronger still!$"

MeteorFalls_1F_2R_Text_NicolasPostRematch:
	.string "So long as you stay strong,\n"
	.string "I can grow stronger too!$"

MeteorFalls_1F_2R_Text_JohnIntro:
	.string "JOHN: My wife and I have battled\n"
	.string "side by side for fifty years.$"

MeteorFalls_1F_2R_Text_JohnDefeat:
	.string "JOHN: Well, dear…\n"
	.string "we had a good run.$"

MeteorFalls_1F_2R_Text_JohnPostBattle:
	.string "JOHN: Fifty years married, and I\n"
	.string "have yet to beat her once.\p"
	.string "We came to HOENN when FALLARBOR\n"
	.string "still grew wheat. Then the ash came.$"

MeteorFalls_1F_2R_Text_JohnNotEnoughMons:
	.string "JOHN: Well, well, a young TRAINER!\p"
	.string "We battle two at a time. Come back\n"
	.string "with more POKéMON, won't you?$"

MeteorFalls_1F_2R_Text_JohnRegister:
	.string "JOHN: Young TRAINER, if the chance\n"
	.string "arises, will you battle with us again?$"

MeteorFalls_1F_2R_Text_JayIntro:
	.string "JAY: Fifty years married, dear.\p"
	.string "We've weathered drought, ash and\n"
	.string "moving house. A battle is nothing.$"

MeteorFalls_1F_2R_Text_JayDefeat:
	.string "JAY: Oh, dear husband.\n"
	.string "We've been bested.$"

MeteorFalls_1F_2R_Text_JayPostBattle:
	.string "JAY: We settle arguments with\n"
	.string "POKéMON battles. Saves plates.$"

MeteorFalls_1F_2R_Text_JayNotEnoughMons:
	.string "JAY: Only one POKéMON, dear?\p"
	.string "We battle as a pair. Come back\n"
	.string "with two.$"

MeteorFalls_1F_2R_Text_JohnRematchIntro:
	.string "JOHN: Back for more, young one?\n"
	.string "We're not done yet!$"

MeteorFalls_1F_2R_Text_JohnRematchDefeat:
	.string "JOHN: Beaten again, dear.\n"
	.string "We're getting old.$"

MeteorFalls_1F_2R_Text_JohnPostRematch:
	.string "JOHN: We lost the farm, but never\n"
	.string "each other.\p"
	.string "You'll learn that's the harvest\n"
	.string "that counts.$"

MeteorFalls_1F_2R_Text_JohnRematchNotEnoughMons:
	.string "JOHN: Well, well, a young TRAINER!\p"
	.string "We battle two at a time. Come back\n"
	.string "with more POKéMON, won't you?$"

MeteorFalls_1F_2R_Text_JayRematchIntro:
	.string "JAY: Fifty years, and we've held\n"
	.string "each other up the whole way.\p"
	.string "That makes a couple strong.$"

MeteorFalls_1F_2R_Text_JayRematchDefeat:
	.string "JAY: Bested again!\n"
	.string "What a strong young one.$"

MeteorFalls_1F_2R_Text_JayPostRematch:
	.string "JAY: Our daughter wants us to move\n"
	.string "in with her in RUSTBORO.\p"
	.string "There's no room, of course.\n"
	.string "There never is anymore.$"

MeteorFalls_1F_2R_Text_JayRematchNotEnoughMons:
	.string "JAY: Only one POKéMON, dear?\p"
	.string "We battle as a pair. Come back\n"
	.string "with two.$"

