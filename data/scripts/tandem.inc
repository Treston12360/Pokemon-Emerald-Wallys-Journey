@ Project Tandem (docs/TANDEM_TASKS.md, T4): A on a friend - what to do together (Brennan 2026-11-27: "trade or battle
@ anywhere by facing a friend, with a menu: trade, player card, battle, team up"). The card, trading (T5) and battles
@ (T6a) work; Team Up comes with T6b.
@ VAR_0x8005 while asking: what for - 0 a trade, 1 a Single Battle, 2 a Double Battle (TANDEM_ACTIVITY_*,
@ include/tandem_link.h).
Tandem_EventScript_Friend::
	lock
	special Tandem_BufferFriendName
	message Tandem_Text_WhatToDo
	waitmessage
	dynmultichoice 0, 0, FALSE, 5, 0, DYN_MULTICHOICE_CB_NONE, Tandem_Text_Trade, Tandem_Text_PlayerCard, Tandem_Text_Battle, Tandem_Text_TeamUp, Tandem_Text_Cancel
	switch VAR_RESULT
	case 0, Tandem_EventScript_Trade
	case 1, Tandem_EventScript_PlayerCard
	case 2, Tandem_EventScript_Battle
	case 3, Tandem_EventScript_NotYet
	closemessage
	release
	end

@ T5: trading - the game's own trade screen, over the network (src/tandem_link.c)
Tandem_EventScript_Trade::
	setvar VAR_0x8005, 0
	goto Tandem_EventScript_AskFriend

@ T6a: a battle - the one asking picks Single or Double (Brennan 2026-11-27 21:20: "Choose each time"); the Cable Club's
@ link battle, everyone at full health, the Pokemon put back as they were after ("Full, then put back")
Tandem_EventScript_Battle::
	message Tandem_Text_WhichBattle
	waitmessage
	dynmultichoice 0, 0, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, Tandem_Text_SingleBattle, Tandem_Text_DoubleBattle, Tandem_Text_Cancel
	switch VAR_RESULT
	case 0, Tandem_EventScript_BattleSingle
	case 1, Tandem_EventScript_BattleDouble
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_BattleSingle::
	setvar VAR_0x8005, 1
	goto Tandem_EventScript_AskFriend

Tandem_EventScript_BattleDouble::
	setvar VAR_0x8005, 2
	special Tandem_CanBattle
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_NeedTwo
	goto Tandem_EventScript_AskFriend

Tandem_EventScript_NeedTwo::
	msgbox Tandem_Text_NeedTwo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

@ ask the friend (VAR_0x8005: what for); they get the question (Tandem_EventScript_Asked); both yes: the trade screen
@ or the battle. Back here, where they stood, when it ends.
Tandem_EventScript_AskFriend::
	special Tandem_Ask
	call Tandem_EventScript_AskingMessage
	waitmessage
	special Tandem_WaitAnswer
	waitstate
	switch VAR_RESULT
	case 0, Tandem_EventScript_SaidNo
	case 2, Tandem_EventScript_Busy
	case 3, Tandem_EventScript_AskEnd
	case 4, Tandem_EventScript_FriendNeedsTwo
	closemessage
	special Tandem_Start
	waitstate
	release
	end

Tandem_EventScript_AskingMessage::
	goto_if_ne VAR_0x8005, 0, Tandem_EventScript_AskingBattleMessage
	message Tandem_Text_AskingTrade
	return

Tandem_EventScript_AskingBattleMessage::
	message Tandem_Text_AskingBattle
	return

Tandem_EventScript_SaidNo::
	goto_if_ne VAR_0x8005, 0, Tandem_EventScript_BattleSaidNo
	msgbox Tandem_Text_TradeSaidNo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_BattleSaidNo::
	msgbox Tandem_Text_BattleSaidNo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_Busy::
	msgbox Tandem_Text_Busy, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_FriendNeedsTwo::
	msgbox Tandem_Text_FriendNeedsTwo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_AskEnd::
	closemessage
	release
	end

@ a friend asked me (started by src/tandem_link.c once I'm free in the overworld)
Tandem_EventScript_Asked::
	lockall
	special Tandem_BufferAsker
	playse SE_PIN
	call Tandem_EventScript_Question
	special Tandem_Answer
	goto_if_eq VAR_RESULT, 2, Tandem_EventScript_AskerGaveUp
	goto_if_eq VAR_RESULT, 3, Tandem_EventScript_INeedTwo
	goto_if_ne VAR_RESULT, YES, Tandem_EventScript_AskedEnd
	message Tandem_Text_GettingReady
	waitmessage
	special Tandem_WaitGo
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_FellThrough
	closemessage
	special Tandem_Start
	waitstate
	releaseall
	end

Tandem_EventScript_Question::
	goto_if_eq VAR_0x8005, 1, Tandem_EventScript_QuestionSingle
	goto_if_eq VAR_0x8005, 2, Tandem_EventScript_QuestionDouble
	msgbox Tandem_Text_WantsToTrade, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionSingle::
	msgbox Tandem_Text_WantsSingle, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionDouble::
	msgbox Tandem_Text_WantsDouble, MSGBOX_YESNO
	return

Tandem_EventScript_AskerGaveUp::
	msgbox Tandem_Text_AskerGaveUp, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

Tandem_EventScript_INeedTwo::
	msgbox Tandem_Text_NeedTwo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

Tandem_EventScript_FellThrough::
	msgbox Tandem_Text_FellThrough, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

Tandem_EventScript_AskedEnd::
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

Tandem_Text_Busy:
	.string "{STR_VAR_1} is busy right now.\n"
	.string "Try again in a moment!$"

Tandem_Text_WhichBattle:
	.string "Which kind of battle?$"

Tandem_Text_SingleBattle:
	.string "Single Battle$"

Tandem_Text_DoubleBattle:
	.string "Double Battle$"

Tandem_Text_NeedTwo:
	.string "You need two Pokémon that can battle\n"
	.string "for a Double Battle.$"

Tandem_Text_AskingBattle:
	.string "Asking {STR_VAR_1} to battle…\n"
	.string "(B Button: stop waiting)$"

Tandem_Text_BattleSaidNo:
	.string "{STR_VAR_1} doesn't want to battle\n"
	.string "right now.$"

Tandem_Text_FriendNeedsTwo:
	.string "{STR_VAR_1} doesn't have two Pokémon\n"
	.string "that can battle.$"

Tandem_Text_WantsSingle:
	.string "{STR_VAR_1} wants a Single Battle.\n"
	.string "Will you battle?$"

Tandem_Text_WantsDouble:
	.string "{STR_VAR_1} wants a Double Battle.\n"
	.string "Will you battle?$"

Tandem_Text_WantsToTrade:
	.string "{STR_VAR_1} wants to trade.\n"
	.string "Will you trade?$"

Tandem_Text_GettingReady:
	.string "Getting ready…$"

Tandem_Text_AskerGaveUp:
	.string "{STR_VAR_1} stopped waiting.$"

Tandem_Text_FellThrough:
	.string "It didn't start.\n"
	.string "Try asking again!$"

Tandem_Text_NoCard:
	.string "Your friend's card hasn't arrived\n"
	.string "yet. Try again in a moment!$"
