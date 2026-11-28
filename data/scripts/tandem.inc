@ Project Tandem (docs/TANDEM_TASKS.md, T4): A on a friend - what to do together (Brennan 2026-11-27: "trade or battle
@ anywhere by facing a friend, with a menu: trade, player card, battle, team up"). The card, trading (T5), battles
@ (T6a) and Team Up (T6b) work.
@ VAR_0x8005 while asking: what for - 0 a trade, 1 a Single Battle, 2 a Double Battle, 3 team up (partners), 4 join a
@ partner's trainer battle (TANDEM_ACTIVITY_*, include/tandem_link.h).
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
	case 3, Tandem_EventScript_TeamUp
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
	goto_if_eq VAR_0x8005, 3, Tandem_EventScript_NowPartners
	closemessage
	special Tandem_Start
	waitstate
	release
	end

Tandem_EventScript_AskingMessage::
	goto_if_eq VAR_0x8005, 3, Tandem_EventScript_AskingTeamUpMessage
	goto_if_ne VAR_0x8005, 0, Tandem_EventScript_AskingBattleMessage
	message Tandem_Text_AskingTrade
	return

Tandem_EventScript_AskingTeamUpMessage::
	message Tandem_Text_AskingTeamUp
	return

Tandem_EventScript_AskingBattleMessage::
	message Tandem_Text_AskingBattle
	return

Tandem_EventScript_SaidNo::
	goto_if_eq VAR_0x8005, 3, Tandem_EventScript_TeamUpSaidNo
	goto_if_ne VAR_0x8005, 0, Tandem_EventScript_BattleSaidNo
	msgbox Tandem_Text_TradeSaidNo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_BattleSaidNo::
	msgbox Tandem_Text_BattleSaidNo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_TeamUpSaidNo::
	msgbox Tandem_Text_TeamUpSaidNo, MSGBOX_DEFAULT
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
	goto_if_eq VAR_0x8005, 3, Tandem_EventScript_AskedNowPartners
	closemessage
	special Tandem_Start
	waitstate
	special Tandem_TeamUpAfter
	call_if_eq VAR_RESULT, 1, Tandem_EventScript_TeamUpExp
	releaseall
	end

Tandem_EventScript_Question::
	goto_if_eq VAR_0x8005, 1, Tandem_EventScript_QuestionSingle
	goto_if_eq VAR_0x8005, 2, Tandem_EventScript_QuestionDouble
	goto_if_eq VAR_0x8005, 3, Tandem_EventScript_QuestionTeamUp
	goto_if_eq VAR_0x8005, 4, Tandem_EventScript_QuestionJoin
	msgbox Tandem_Text_WantsToTrade, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionSingle::
	msgbox Tandem_Text_WantsSingle, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionDouble::
	msgbox Tandem_Text_WantsDouble, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionTeamUp::
	msgbox Tandem_Text_WantsTeamUp, MSGBOX_YESNO
	return

Tandem_EventScript_QuestionJoin::
	msgbox Tandem_Text_JoinTheBattle, MSGBOX_YESNO
	return

Tandem_EventScript_AskerGaveUp::
	msgbox Tandem_Text_AskerGaveUp, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

Tandem_EventScript_INeedTwo::
	goto_if_eq VAR_0x8005, 4, Tandem_EventScript_INeedOne
	msgbox Tandem_Text_NeedTwo, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

Tandem_EventScript_INeedOne::
	msgbox Tandem_Text_NoneCanBattle, MSGBOX_DEFAULT
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

@ T6b: Team Up on a friend - partners (Brennan 2026-11-27 23:3x "Partner up first"); on my partner again: stop
Tandem_EventScript_TeamUp::
	special Tandem_PartnerState
	goto_if_eq VAR_RESULT, TRUE, Tandem_EventScript_StopTeamUp
	setvar VAR_0x8005, 3
	goto Tandem_EventScript_AskFriend

Tandem_EventScript_StopTeamUp::
	msgbox Tandem_Text_StopTeamUp, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Tandem_EventScript_AskEnd
	special Tandem_StopPartner
	msgbox Tandem_Text_StoppedTeamUp, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_NowPartners::
	special Tandem_SetPartner
	msgbox Tandem_Text_NowPartners, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskEnd

Tandem_EventScript_AskedNowPartners::
	special Tandem_SetPartner
	msgbox Tandem_Text_NowPartners, MSGBOX_DEFAULT
	goto Tandem_EventScript_AskedEnd

@ my partner stopped (src/tandem_link.c, once I'm free)
Tandem_EventScript_PartnerEnded::
	lockall
	msgbox Tandem_Text_PartnerEnded, MSGBOX_DEFAULT
	releaseall
	end

@ T6b: before every trainer battle (data/scripts/trainer_battle.inc EventSnippet_DoTrainerBattle): my partner on my
@ screen and a regular trainer - they are asked "join the battle?" (Brennan: "Ask each time"); yes: the battle is a
@ team-up (battle_setup.c); no, busy or no answer: as always
Tandem_EventScript_BeforeTrainerBattle::
	special Tandem_TeamUpCheck
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_TeamUpReturn
	special Tandem_TeamUpAsk
	message Tandem_Text_AskingJoin
	waitmessage
	special Tandem_WaitAnswer
	waitstate
	goto_if_eq VAR_RESULT, 1, Tandem_EventScript_TeamUpJoins
	goto_if_eq VAR_RESULT, 3, Tandem_EventScript_TeamUpClose
	msgbox Tandem_Text_CantJoin, MSGBOX_DEFAULT
Tandem_EventScript_TeamUpClose::
	closemessage
Tandem_EventScript_TeamUpReturn::
	return

Tandem_EventScript_TeamUpJoins::
	closemessage
	special Tandem_TeamUpGo
	return

@ ... and after it: a won team-up - the Exp.; one cut short (the connection broke) - the trainer isn't beaten: the
@ trainer's script ends here (it spots me again)
Tandem_EventScript_AfterTrainerBattle::
	special Tandem_TeamUpAfter
	goto_if_eq VAR_RESULT, 2, Tandem_EventScript_TeamUpBroke
	call_if_eq VAR_RESULT, 1, Tandem_EventScript_TeamUpExp
	return

Tandem_EventScript_TeamUpBroke::
	releaseall
	end

@ T6b: Exp. for both (Brennan: "Exp. for both, rest for one") - each game its own Pokemon's, after the battle
@ (src/tandem_link.c): the battle's own lines, new moves, then evolutions
Tandem_EventScript_TeamUpExp::
	special Tandem_TeamUpExpStart
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_TeamUpEvolve
Tandem_EventScript_TeamUpExpLoop::
	special Tandem_TeamUpExpNext
	switch VAR_RESULT
	case 1, Tandem_EventScript_ExpGained
	case 2, Tandem_EventScript_ExpRestAll
	case 3, Tandem_EventScript_ExpRestShare
	case 4, Tandem_EventScript_ExpGrew
	case 5, Tandem_EventScript_ExpGrewPoints
	case 6, Tandem_EventScript_ExpLearned
	case 7, Tandem_EventScript_ExpWantsMove
Tandem_EventScript_TeamUpEvolve::
	closemessage
	special Tandem_TeamUpEvolve
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_TeamUpExpEnd
	waitstate
	goto Tandem_EventScript_TeamUpEvolve
Tandem_EventScript_TeamUpExpEnd::
	return

Tandem_EventScript_ExpGained::
	msgbox Tandem_Text_ExpGained, MSGBOX_DEFAULT
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpRestAll::
	msgbox Tandem_Text_ExpRestAll, MSGBOX_DEFAULT
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpRestShare::
	msgbox Tandem_Text_ExpRestShare, MSGBOX_DEFAULT
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpGrew::
	playfanfare MUS_LEVEL_UP
	message Tandem_Text_ExpGrew
	waitmessage
	waitfanfare
	waitbuttonpress
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpGrewPoints::
	playfanfare MUS_LEVEL_UP
	message Tandem_Text_ExpGrewPoints
	waitmessage
	waitfanfare
	waitbuttonpress
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpLearned::
	playfanfare MUS_LEVEL_UP
	message Tandem_Text_ExpLearned
	waitmessage
	waitfanfare
	waitbuttonpress
	goto Tandem_EventScript_TeamUpExpLoop

@ four moves already: forget one? (the summary screen, as the Move Deleter opens it)
Tandem_EventScript_ExpWantsMove::
	msgbox Tandem_Text_ExpWantsMove, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Tandem_EventScript_ExpDidNotLearn
	fadescreen FADE_TO_BLACK
	special Tandem_TeamUpForgetMove
	fadescreen FADE_FROM_BLACK
	special Tandem_TeamUpForgotMove
	goto_if_eq VAR_RESULT, FALSE, Tandem_EventScript_ExpDidNotLearn
	msgbox Tandem_Text_ExpForgot, MSGBOX_DEFAULT
	goto Tandem_EventScript_TeamUpExpLoop

Tandem_EventScript_ExpDidNotLearn::
	special Tandem_TeamUpSkipMove
	msgbox Tandem_Text_ExpDidNotLearn, MSGBOX_DEFAULT
	goto Tandem_EventScript_TeamUpExpLoop

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

Tandem_Text_AskingTeamUp:
	.string "Asking {STR_VAR_1} to team up…\n"
	.string "(B Button: stop waiting)$"

Tandem_Text_TeamUpSaidNo:
	.string "{STR_VAR_1} doesn't want to team up\n"
	.string "right now.$"

Tandem_Text_WantsTeamUp:
	.string "{STR_VAR_1} wants to team up!\p"
	.string "When a trainer spots one of you,\n"
	.string "the other can join the battle.\p"
	.string "Team up with {STR_VAR_1}?$"

Tandem_Text_NowPartners:
	.string "You and {STR_VAR_1} are partners now!\p"
	.string "When a trainer spots one of you,\n"
	.string "the other can join the battle.$"

Tandem_Text_StopTeamUp:
	.string "Stop teaming up with {STR_VAR_1}?$"

Tandem_Text_StoppedTeamUp:
	.string "You stopped teaming up\n"
	.string "with {STR_VAR_1}.$"

Tandem_Text_PartnerEnded:
	.string "{STR_VAR_1} stopped teaming up\n"
	.string "with you.$"

Tandem_Text_AskingJoin:
	.string "Calling {STR_VAR_1} to join…\n"
	.string "(B Button: battle alone)$"

Tandem_Text_CantJoin:
	.string "{STR_VAR_1} can't join\n"
	.string "this time.$"

Tandem_Text_JoinTheBattle:
	.string "{STR_VAR_1} was spotted by\n"
	.string "{STR_VAR_2}!\p"
	.string "Join the battle?$"

Tandem_Text_NoneCanBattle:
	.string "You don't have a Pokémon\n"
	.string "that can battle.$"

Tandem_Text_ExpGained:
	.string "{STR_VAR_1} gained{STR_VAR_3} {STR_VAR_2}\n"
	.string "Exp. Points!$"

Tandem_Text_ExpRestAll:
	.string "The rest of your team shared\n"
	.string "the Exp. Points!$"

Tandem_Text_ExpRestShare:
	.string "The rest of your team gained Exp.\n"
	.string "Points thanks to the Exp. Share!$"

Tandem_Text_ExpGrew:
	.string "{STR_VAR_1} grew to Lv. {STR_VAR_2}!$"

Tandem_Text_ExpGrewPoints:
	.string "{STR_VAR_1} grew to Lv. {STR_VAR_2}!\n"
	.string "It also gained some EVs!$"

Tandem_Text_ExpLearned:
	.string "{STR_VAR_1} learned {STR_VAR_2}!$"

Tandem_Text_ExpWantsMove:
	.string "{STR_VAR_1} wants to learn the\n"
	.string "move {STR_VAR_2}.\p"
	.string "However, {STR_VAR_1} already\n"
	.string "knows four moves.\p"
	.string "Should another move be forgotten\n"
	.string "and replaced with {STR_VAR_2}?$"

Tandem_Text_ExpForgot:
	.string "One…{PAUSE 10} two…{PAUSE 10} and…{PAUSE 30}{PLAY_SE SE_BALL_BOUNCE_1}\n"
	.string "ta-da!\p"
	.string "{STR_VAR_1} forgot {STR_VAR_3}…\p"
	.string "And…\p"
	.string "{STR_VAR_1} learned {STR_VAR_2}!$"

Tandem_Text_ExpDidNotLearn:
	.string "{STR_VAR_1} did not learn\n"
	.string "{STR_VAR_2}.$"
