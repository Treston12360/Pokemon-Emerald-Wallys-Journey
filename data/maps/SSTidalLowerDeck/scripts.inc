SSTidalLowerDeck_MapScripts::
	.byte 0

SSTidalLowerDeck_EventScript_Phillip::
	trainerbattle_single TRAINER_PHILLIP, SSTidalLowerDeck_Text_PhillipIntro, SSTidalLowerDeck_Text_PhillipDefeat
	msgbox SSTidalLowerDeck_Text_PhillipPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalLowerDeck_EventScript_Leonard::
	trainerbattle_single TRAINER_LEONARD, SSTidalLowerDeck_Text_LeonardIntro, SSTidalLowerDeck_Text_LeonardDefeat
	msgbox SSTidalLowerDeck_Text_LeonardPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalLowerDeck_Text_PhillipIntro:
	.string "Arrgh! I'm sick of scrubbing this\n"
	.string "huge ship! Quick battle!$"

SSTidalLowerDeck_Text_PhillipDefeat:
	.string "Little bro! I lost!\n"
	.string "You're scrubbing alone tonight!$"

SSTidalLowerDeck_Text_PhillipPostBattle:
	.string "We're the CLEANUP BROTHERS!\p"
	.string "Big bro pours the soap, little bro\n"
	.string "does the scrubbing!$"

SSTidalLowerDeck_Text_LeonardIntro:
	.string "Me and my brother scrub this whole\n"
	.string "hull every trip!\p"
	.string "Plenty of room for a battle down\n"
	.string "here!$"

SSTidalLowerDeck_Text_LeonardDefeat:
	.string "Big bro! I lost!\n"
	.string "Don't make me scrub more!$"

SSTidalLowerDeck_Text_LeonardPostBattle:
	.string "We're the CLEANUP BROTHERS!\p"
	.string "Big bro pours the soap, little bro\n"
	.string "does the scrubbing!$"

