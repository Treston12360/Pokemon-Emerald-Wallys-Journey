RustboroCity_House2_MapScripts::
	.byte 0

RustboroCity_House2_EventScript_PokefanF::
	msgbox RustboroCity_House2_Text_TrainerSchoolExcellent, MSGBOX_NPC
	end

RustboroCity_House2_EventScript_LittleGirl::
	goto_if_set FLAG_DEFEATED_RUSTBORO_GYM, RustboroCity_House2_Text_RoxanneKnowsALot_W0_Script
	msgbox RustboroCity_House2_Text_RoxanneKnowsALot, MSGBOX_NPC
	end

RustboroCity_House2_Text_RoxanneKnowsALot_W0_Script:
	msgbox RustboroCity_House2_Text_RoxanneKnowsALot_W0, MSGBOX_NPC
	end

RustboroCity_House2_Text_TrainerSchoolExcellent:
	.string "The Trainer's School is excellent.\p"
	.string "Roxanne teaches there between Gym\n"
	.string "challenges. She says a Leader who\l"
	.string "stops learning stops leading.$"

RustboroCity_House2_Text_RoxanneKnowsALot:
	.string "Roxanne, the Gym Leader, really knows\n"
	.string "a lot about Pokémon.\p"
	.string "She's really strong, too!$"

RustboroCity_House2_Text_RoxanneKnowsALot_W0:
	.string "Roxanne told her class about your\n"
	.string "battle today.\p"
	.string "“Observe how they adapted,” she said.\n"
	.string "“That is the lesson.”$"
