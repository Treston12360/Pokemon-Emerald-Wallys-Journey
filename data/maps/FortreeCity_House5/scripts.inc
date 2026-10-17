FortreeCity_House5_MapScripts::
	.byte 0

FortreeCity_House5_EventScript_PokefanF::
	msgbox FortreeCity_House5_Text_TreeHousesAreGreat, MSGBOX_NPC
	end

FortreeCity_House5_EventScript_Man::
	goto_if_set FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, FortreeCity_House5_Text_AdaptedToNature_W0_Script
	msgbox FortreeCity_House5_Text_AdaptedToNature, MSGBOX_NPC
	end

FortreeCity_House5_Text_AdaptedToNature_W0_Script:
	msgbox FortreeCity_House5_Text_AdaptedToNature_W0, MSGBOX_NPC
	end

FortreeCity_House5_EventScript_Zigzagoon::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox FortreeCity_House5_Text_Zigzagoon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

FortreeCity_House5_Text_TreeHousesAreGreat:
	.string "FORTREE's tree houses are the best\n"
	.string "place to live with POKéMON.\p"
	.string "We built around the trees instead\n"
	.string "of cutting them down. Took longer.\l"
	.string "Worth it.$"

FortreeCity_House5_Text_AdaptedToNature:
	.string "People and POKéMON adapted to nature\n"
	.string "to survive.\p"
	.string "TEAM MAGMA wants nature to adapt to\n"
	.string "us. TEAM AQUA wants us to disappear.\p"
	.string "There's a middle. Nobody wants to\n"
	.string "live there.$"

FortreeCity_House5_Text_Zigzagoon:
	.string "ZIGZAGOON: Bufuu!$"

FortreeCity_House5_Text_AdaptedToNature_W0:
	.string "This rain isn't natural. The trees\n"
	.string "know it.\p"
	.string "Listen. Not one BIRD POKéMON is\n"
	.string "singing.$"
