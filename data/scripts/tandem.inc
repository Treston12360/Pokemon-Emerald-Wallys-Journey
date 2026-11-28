@ Project Tandem (docs/TANDEM_TASKS.md, T4): A on a friend - what to do together (Brennan 2026-11-27: "trade or battle
@ anywhere by facing a friend, with a menu: trade, player card, battle, team up"). The card and trading (T5) work;
@ battle and team up come with T6.
Tandem_EventScript_Friend::
	lock
	special Tandem_BufferFriendName
	message Tandem_Text_WhatToDo
	waitmessage
	dynmultichoice 0, 0, FALSE, 5, 0, DYN_MULTICHOICE_CB_NONE, Tandem_Text_Trade, Tandem_Text_PlayerCard, Tandem_Text_Battle, Tandem_Text_TeamUp, Tandem_Text_Cancel
	switch VAR_RESULT
	case 0, Tandem_EventScript_Trade
	case 1, Tandem_EventScript_PlayerCard
	case 2, Tandem_EventScript_NotYet
	case 3, Tandem_EventScript_NotYet
	closemessage
	release
	end

@ T5: trading. Ask the friend; they get "NAME wants to trade" (Tandem_EventScript_TradeAsked); both yes: the game's
@ own trade screen, over the network (src/tandem_link.c). Back here, where they stood, when it closes.
Tandem_EventScript_Trade::
	special Tandem_AskTrade
	message Tandem_Text_AskingTrade
	waitmessage
	special Tandem_WaitTradeAnswer
	waitstate
	switch VAR_RESULT
	case 0, Tandem_EventScript_TradeSaidNo
	case 2, Tandem_EventScript_TradeBusy
	case 3, Tandem_EventScript_TradeEnd
	closemessage
	special Tandem_StartTrade
	waitstate
	release
	end

Tandem_EventScript_TradeSaidNo::
	msgbox Tandem_Text_TradeSaidNo, MSGBOX_DEFAULT
	goto Tandem_EventScript_TradeEnd

Tandem_EventScript_TradeBusy::
	msgbox Tandem_Text_TradeBusy, MSGBOX_DEFAULT
	goto Tandem_EventScript_TradeEnd

Tandem_EventScript_TradeEnd::
	closemessage
	release
	end

@ a friend asked me (started by src/tandem_link.c once I'm free in the overworld)
Tandem_EventScript_TradeAsked::
	lockall
	special Tandem_BufferAsker
	playse SE_PIN
	msgbox Tandem_Text_WantsToTrade, MSGBOX_YESNO
	special Tandem_AnswerTrade
	goto_if_eq VAR_RESULT, 2, Tandem_EventScript_AskerGaveUp
	goto_if_ne VAR_RESULT, YES, Tandem_EventScript_TradeAskedEnd
	message Tandem_Text_TradeStarting
	waitmessage
	special Tandem_WaitTradeGo
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_TradeFellThrough
	closemessage
	special Tandem_StartTrade
	waitstate
	releaseall
	end

Tandem_EventScript_AskerGaveUp::
	msgbox Tandem_Text_AskerGaveUp, MSGBOX_DEFAULT
	goto Tandem_EventScript_TradeAskedEnd

Tandem_EventScript_TradeFellThrough::
	msgbox Tandem_Text_TradeFellThrough, MSGBOX_DEFAULT
	goto Tandem_EventScript_TradeAskedEnd

Tandem_EventScript_TradeAskedEnd::
	closemessage
	releaseall
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

Tandem_Text_AskingTrade:
	.string "Asking {STR_VAR_1} to trade…\n"
	.string "(B Button: stop waiting)$"

Tandem_Text_TradeSaidNo:
	.string "{STR_VAR_1} doesn't want to trade\n"
	.string "right now.$"

Tandem_Text_TradeBusy:
	.string "{STR_VAR_1} can't trade right now.\n"
	.string "Try again in a moment!$"

Tandem_Text_WantsToTrade:
	.string "{STR_VAR_1} wants to trade.\n"
	.string "Will you trade?$"

Tandem_Text_TradeStarting:
	.string "Getting ready to trade…$"

Tandem_Text_AskerGaveUp:
	.string "{STR_VAR_1} stopped waiting.$"

Tandem_Text_TradeFellThrough:
	.string "The trade didn't start.\n"
	.string "Try asking again!$"

Tandem_Text_NoCard:
	.string "Your friend's card hasn't arrived\n"
	.string "yet. Try again in a moment!$"
