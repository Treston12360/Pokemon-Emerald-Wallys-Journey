@ Project Tandem (docs/TANDEM_TASKS.md, T4): A on a friend - what to do together (Brennan 2026-11-27: "trade or battle
@ anywhere by facing a friend, with a menu: trade, player card, battle, team up"). The card works now; trade, battle and
@ team up come with T5 and T6.
Tandem_EventScript_Friend::
	lock
	special Tandem_BufferFriendName
	message Tandem_Text_WhatToDo
	waitmessage
	dynmultichoice 0, 0, FALSE, 5, 1, DYN_MULTICHOICE_CB_NONE, Tandem_Text_Trade, Tandem_Text_PlayerCard, Tandem_Text_Battle, Tandem_Text_TeamUp, Tandem_Text_Cancel
	switch VAR_RESULT
	case 0, Tandem_EventScript_NotYet
	case 1, Tandem_EventScript_PlayerCard
	case 2, Tandem_EventScript_NotYet
	case 3, Tandem_EventScript_NotYet
	closemessage
	release
	end

Tandem_EventScript_NotYet::
	msgbox Tandem_Text_NotYet, MSGBOX_DEFAULT
	closemessage
	release
	end

Tandem_EventScript_PlayerCard::
	goto_if_eq VAR_0x8004, FALSE, Tandem_EventScript_NoCard
	closemessage
	fadescreen FADE_TO_BLACK
	special Tandem_ShowFriendCard
	waitstate
	release
	end

Tandem_EventScript_NoCard::
	msgbox Tandem_Text_NoCard, MSGBOX_DEFAULT
	closemessage
	release
	end

Tandem_Text_WhatToDo:
	.string "What would you like to do\n"
	.string "with {STR_VAR_1}?$"

Tandem_Text_Trade:
	.string "Trade$"

Tandem_Text_PlayerCard:
	.string "Player Card$"

Tandem_Text_Battle:
	.string "Battle$"

Tandem_Text_TeamUp:
	.string "Team Up$"

Tandem_Text_Cancel:
	.string "Cancel$"

Tandem_Text_NotYet:
	.string "That isn't ready yet.\n"
	.string "It's coming in a later update!$"

Tandem_Text_NoCard:
	.string "Your friend's card hasn't arrived\n"
	.string "yet. Try again in a moment!$"
