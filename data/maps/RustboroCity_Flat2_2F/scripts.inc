RustboroCity_Flat2_2F_MapScripts::
	.byte 0

RustboroCity_Flat2_2F_EventScript_OldMan::
	msgbox RustboroCity_Flat2_2F_Text_DevonWasTinyInOldDays, MSGBOX_NPC
	end

RustboroCity_Flat2_2F_EventScript_NinjaBoy::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_PREMIER_BALL_RUSTBORO, RustboroCity_Flat2_2F_EventScript_GavePremierBall
	msgbox RustboroCity_Flat2_2F_Text_MyDaddyMadeThisYouCanHaveIt, MSGBOX_DEFAULT
	giveitem ITEM_POKE_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_PREMIER_BALL_RUSTBORO
	release
	end

RustboroCity_Flat2_2F_EventScript_GavePremierBall::
	msgbox RustboroCity_Flat2_2F_Text_GoingToWorkAtDevonToo, MSGBOX_DEFAULT
	release
	end

RustboroCity_Flat2_2F_Text_DevonWasTinyInOldDays:
	.string "Devon started as a tiny workshop.\n"
	.string "Mr. Stone fixing Poké Ball hinges.\p"
	.string "Now half of Hoenn carries something\n"
	.string "he made. He earned that.\p"
	.string "But he built the factory by the\n"
	.string "river. The river never asked to\l"
	.string "be part of it.$"

RustboroCity_Flat2_2F_Text_MyDaddyMadeThisYouCanHaveIt:
	.string "My daddy's working at the Corporation.\p"
	.string "My daddy made this!\n"
	.string "But I can't use it, so you can have it.$"

RustboroCity_Flat2_2F_Text_GoingToWorkAtDevonToo:
	.string "My daddy works at Devon.\p"
	.string "When I grow up, I'll work there too.\n"
	.string "Then I'll be the boss and fix the\l"
	.string "river.$"

