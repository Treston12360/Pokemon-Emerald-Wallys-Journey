Route119_House_MapScripts::
	.byte 0

Route119_House_EventScript_Woman::
	msgbox Route119_House_Text_RumorAboutCaveOfOrigin, MSGBOX_NPC
	end

Route119_House_EventScript_Wingull::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_WINGULL, CRY_MODE_NORMAL
	msgbox Route119_House_Text_Wingull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Route119_House_Text_RumorAboutCaveOfOrigin:
	.string "They say there's a CAVE OF ORIGIN\n"
	.string "under SOOTOPOLIS.\p"
	.string "Old stories call it the place where\n"
	.string "the land and sea were first born.\p"
	.string "And where they'd fight again, if\n"
	.string "anyone woke them.$"

Route119_House_Text_Wingull:
	.string "WINGULL: Pihyoh!$"

