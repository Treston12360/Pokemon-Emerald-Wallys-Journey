LilycoveCity_House1_MapScripts::
	.byte 0

LilycoveCity_House1_EventScript_ExpertM::
	goto_if_set FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, LilycoveCity_House1_Text_PokemonPartnersNotTools_W0_Script
	msgbox LilycoveCity_House1_Text_PokemonPartnersNotTools, MSGBOX_NPC
	end

LilycoveCity_House1_Text_PokemonPartnersNotTools_W0_Script:
	msgbox LilycoveCity_House1_Text_PokemonPartnersNotTools_W0, MSGBOX_NPC
	end

LilycoveCity_House1_EventScript_Kecleon::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_KECLEON, CRY_MODE_NORMAL
	msgbox LilycoveCity_House1_Text_Kecleon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

LilycoveCity_House1_Text_PokemonPartnersNotTools:
	.string "POKéMON are partners. Not tools.\p"
	.string "TEAM AQUA says they love POKéMON.\n"
	.string "Then they use them to steal.\p"
	.string "Love isn't what you say. It's what\n"
	.string "you do.$"

LilycoveCity_House1_Text_Kecleon:
	.string "KECLEON: Ruroro?$"

LilycoveCity_House1_Text_PokemonPartnersNotTools_W0:
	.string "TEAM AQUA used their WAILMER as\n"
	.string "battering rams in the cove.\p"
	.string "Partners, not tools. I'll keep\n"
	.string "saying it until someone listens.$"
