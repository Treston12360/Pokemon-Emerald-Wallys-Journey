RustboroCity_House3_MapScripts::
	.byte 0

RustboroCity_House3_EventScript_OldMan::
	msgbox RustboroCity_House3_Text_IGivePerfectlySuitedNicknames, MSGBOX_NPC
	end

RustboroCity_House3_EventScript_OldWoman::
	msgbox RustboroCity_House3_Text_NamingPikachuPekachu, MSGBOX_NPC
	end

@ Misspelling on purpose, see nickname
RustboroCity_House3_EventScript_Pekachu::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIKACHU, CRY_MODE_NORMAL
	msgbox RustboroCity_House3_Text_Pekachu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

RustboroCity_House3_Text_IGivePerfectlySuitedNicknames:
	.string "I give my POKéMON perfect nicknames!\p"
	.string "A name says you see them. Not just\n"
	.string "what they are. Who they are.$"

RustboroCity_House3_Text_NamingPikachuPekachu:
	.string "But PEKACHU for a PIKACHU?\p"
	.string "…Fine. It's easy to remember. And\n"
	.string "he answers to it. That's what\l"
	.string "matters.$"

RustboroCity_House3_Text_Pekachu:
	.string "PEKACHU: Peka!$"

