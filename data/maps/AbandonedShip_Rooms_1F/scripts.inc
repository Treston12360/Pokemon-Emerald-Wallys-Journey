AbandonedShip_Rooms_1F_MapScripts::
	.byte 0

AbandonedShip_Rooms_1F_EventScript_Gentleman::
	msgbox AbandonedShip_Rooms_1F_Text_TakingALookAround, MSGBOX_NPC
	end

AbandonedShip_Rooms_1F_EventScript_Demetrius::
	trainerbattle_single TRAINER_DEMETRIUS, AbandonedShip_Rooms_1F_Text_DemetriusIntro, AbandonedShip_Rooms_1F_Text_DemetriusDefeat
	msgbox AbandonedShip_Rooms_1F_Text_DemetriusPostBattle, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms_1F_EventScript_Thalia::
	trainerbattle_single TRAINER_THALIA_1, AbandonedShip_Rooms_1F_Text_ThaliaIntro, AbandonedShip_Rooms_1F_Text_ThaliaDefeat, AbandonedShip_Rooms_1F_EventScript_RegisterThalia
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, AbandonedShip_Rooms_1F_EventScript_ThaliaRematch
	msgbox AbandonedShip_Rooms_1F_Text_ThaliaPostBattle, MSGBOX_DEFAULT
	release
	end

AbandonedShip_Rooms_1F_EventScript_RegisterThalia::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	msgbox AbandonedShip_Rooms_1F_Text_ThaliaRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_THALIA_1
	release
	end

AbandonedShip_Rooms_1F_EventScript_ThaliaRematch::
	trainerbattle_rematch TRAINER_THALIA_1, AbandonedShip_Rooms_1F_Text_ThaliaRematchIntro, AbandonedShip_Rooms_1F_Text_ThaliaRematchDefeat
	msgbox AbandonedShip_Rooms_1F_Text_ThaliaPostRematch, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms_1F_Text_TakingALookAround:
	.string "My father was a cabin boy on this\n"
	.string "ship.\p"
	.string "He said the storm came out of a clear\n"
	.string "sky. No warning. Like the sea had\l"
	.string "made up its mind.$"

AbandonedShip_Rooms_1F_Text_ThaliaIntro:
	.string "My grandpa was a deckhand here.\p"
	.string "He said a WAILORD sang the night she\n"
	.string "went down. A warning, he said.$"

AbandonedShip_Rooms_1F_Text_ThaliaDefeat:
	.string "Curious AND strong!$"

AbandonedShip_Rooms_1F_Text_ThaliaPostBattle:
	.string "The man next door says he's just\n"
	.string "sightseeing. I don't buy it.$"

AbandonedShip_Rooms_1F_Text_ThaliaRegister:
	.string "You're such a tough TRAINER!\n"
	.string "Let me register you as a memento!$"

AbandonedShip_Rooms_1F_Text_ThaliaRematchIntro:
	.string "Back again? Still curious about this\n"
	.string "wreck? Me too. Battle!$"

AbandonedShip_Rooms_1F_Text_ThaliaRematchDefeat:
	.string "Too strong!$"

AbandonedShip_Rooms_1F_Text_ThaliaPostRematch:
	.string "That man next door is up to\n"
	.string "something. I just know it!$"

AbandonedShip_Rooms_1F_Text_DemetriusIntro:
	.string "Waaah! Mom found me! …Huh?\p"
	.string "Phew. Mom says ghosts of the old crew\n"
	.string "live here. Battle me so I can hide!$"

AbandonedShip_Rooms_1F_Text_DemetriusDefeat:
	.string "Phew. At least you're\n"
	.string "not my mom.$"

AbandonedShip_Rooms_1F_Text_DemetriusPostBattle:
	.string "I ran away because Mom's mad.\n"
	.string "Don't tell her where I am!$"

