DewfordTown_House1_MapScripts::
	.byte 0

DewfordTown_House1_EventScript_Man::
	msgbox DewfordTown_House1_Text_LotToBeSaidForLivingOnIsland, MSGBOX_NPC
	end

DewfordTown_House1_EventScript_Woman::
	goto_if_set FLAG_DEFEATED_SOOTOPOLIS_GYM, DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W0_Script
	goto_if_set FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W1_Script
	msgbox DewfordTown_House1_Text_LifeGoesSlowlyOnIsland, MSGBOX_NPC
	end

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W0_Script:
	msgbox DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W0, MSGBOX_NPC
	end

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W1_Script:
	msgbox DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W1, MSGBOX_NPC
	end

DewfordTown_House1_EventScript_Zigzagoon::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox DewfordTown_House1_Text_Zigzagoon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

DewfordTown_House1_Text_LotToBeSaidForLivingOnIsland:
	.string "Living on a small island teaches you\n"
	.string "something.\p"
	.string "When the storm comes, nobody's\n"
	.string "coming to save you. So you save\l"
	.string "each other.$"

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland:
	.string "I left the major port of SLATEPORT\n"
	.string "CITY when I married my husband here.\p"
	.string "Life goes by slowly on this little\n"
	.string "island. But being surrounded by the\l"
	.string "beautiful sea--that's happiness, too.$"

DewfordTown_House1_Text_Zigzagoon:
	.string "ZIGZAGOON: Guguuh!$"

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W0:
	.string "My husband came home safe after the\n"
	.string "storm.\p"
	.string "Everyone in DEWFORD hugs a little\n"
	.string "longer now.$"

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland_W1:
	.string "The waves are wrong. Too high, too\n"
	.string "fast.\p"
	.string "My husband's still out on the boat.\n"
	.string "Please, if you see him…$"
