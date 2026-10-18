AbandonedShip_Rooms2_1F_MapScripts::
	.byte 0

AbandonedShip_Rooms2_1F_EventScript_Dan::
	trainerbattle_double TRAINER_KIRA_AND_DAN_1, AbandonedShip_Rooms2_1F_Text_DanIntro, AbandonedShip_Rooms2_1F_Text_DanDefeat, AbandonedShip_Rooms2_1F_Text_DanNotEnoughMons, AbandonedShip_Rooms2_1F_EventScript_RegisterDan
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, AbandonedShip_Rooms2_1F_EventScript_DanRematch
	msgbox AbandonedShip_Rooms2_1F_Text_DanPostBattle, MSGBOX_DEFAULT
	release
	end

AbandonedShip_Rooms2_1F_EventScript_RegisterDan::
	msgbox AbandonedShip_Rooms2_1F_Text_KiraRegister, MSGBOX_DEFAULT  @ Kira speaks for both when registering KiraAndDan
	register_matchcall TRAINER_KIRA_AND_DAN_1
	release
	end

AbandonedShip_Rooms2_1F_EventScript_DanRematch::
	trainerbattle_rematch_double TRAINER_KIRA_AND_DAN_1, AbandonedShip_Rooms2_1F_Text_DanRematchIntro, AbandonedShip_Rooms2_1F_Text_DanRematchDefeat, AbandonedShip_Rooms2_1F_Text_DanRematchNotEnoughMons
	msgbox AbandonedShip_Rooms2_1F_Text_DanPostRematch, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms2_1F_EventScript_Kira::
	trainerbattle_double TRAINER_KIRA_AND_DAN_1, AbandonedShip_Rooms2_1F_Text_KiraIntro, AbandonedShip_Rooms2_1F_Text_KiraDefeat, AbandonedShip_Rooms2_1F_Text_KiraNotEnoughMons, AbandonedShip_Rooms2_1F_EventScript_RegisterKira
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, AbandonedShip_Rooms2_1F_EventScript_KiraRematch
	msgbox AbandonedShip_Rooms2_1F_Text_KiraPostBattle, MSGBOX_DEFAULT
	release
	end

AbandonedShip_Rooms2_1F_EventScript_RegisterKira::
	msgbox AbandonedShip_Rooms2_1F_Text_KiraRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_KIRA_AND_DAN_1
	release
	end

AbandonedShip_Rooms2_1F_EventScript_KiraRematch::
	trainerbattle_rematch_double TRAINER_KIRA_AND_DAN_1, AbandonedShip_Rooms2_1F_Text_KiraRematchIntro, AbandonedShip_Rooms2_1F_Text_KiraRematchDefeat, AbandonedShip_Rooms2_1F_Text_KiraRematchNotEnoughMons
	msgbox AbandonedShip_Rooms2_1F_Text_KiraPostRematch, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms2_1F_EventScript_Jani::
	trainerbattle_single TRAINER_JANI, AbandonedShip_Rooms2_1F_Text_JaniIntro, AbandonedShip_Rooms2_1F_Text_JaniDefeat
	msgbox AbandonedShip_Rooms2_1F_Text_JaniPostBattle, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms2_1F_EventScript_Garrison::
	trainerbattle_single TRAINER_GARRISON, AbandonedShip_Rooms2_1F_Text_GarrisonIntro, AbandonedShip_Rooms2_1F_Text_GarrisonDefeat
	msgbox AbandonedShip_Rooms2_1F_Text_GarrisonPostBattle, MSGBOX_AUTOCLOSE
	end

AbandonedShip_Rooms2_1F_Text_DanIntro:
	.string "DAN: KIRA heard there's treasure on\n"
	.string "this wreck.\p"
	.string "If we find it, we can stop renting.\n"
	.string "Instead we found you!$"

AbandonedShip_Rooms2_1F_Text_DanDefeat:
	.string "DAN: Even together we couldn't\n"
	.string "win…$"

AbandonedShip_Rooms2_1F_Text_DanPostBattle:
	.string "DAN: No treasures anywhere…\n"
	.string "Did someone beat us to them?$"

AbandonedShip_Rooms2_1F_Text_DanNotEnoughMons:
	.string "DAN: You need two POKéMON to\n"
	.string "battle us both.$"

AbandonedShip_Rooms2_1F_Text_KiraIntro:
	.string "KIRA: Treasure means a house of our\n"
	.string "own! Out of the way, TRAINER!$"

AbandonedShip_Rooms2_1F_Text_KiraDefeat:
	.string "KIRA: Ooh! There goes our\n"
	.string "treasure luck!$"

AbandonedShip_Rooms2_1F_Text_KiraPostBattle:
	.string "KIRA: Where could the treasures be?\p"
	.string "If we find them, we're putting a\n"
	.string "down payment on a house!$"

AbandonedShip_Rooms2_1F_Text_KiraNotEnoughMons:
	.string "KIRA: Only one POKéMON?\n"
	.string "We need two to battle!$"

AbandonedShip_Rooms2_1F_Text_KiraRegister:
	.string "KIRA: Oh, you make me so angry!\n"
	.string "I'm going to register you for that!$"

AbandonedShip_Rooms2_1F_Text_DanRematchIntro:
	.string "DAN: Still treasure hunting.\n"
	.string "Our POKéMON got stronger!$"

AbandonedShip_Rooms2_1F_Text_DanRematchDefeat:
	.string "DAN: Strong as ever!$"

AbandonedShip_Rooms2_1F_Text_DanPostRematch:
	.string "DAN: No treasure, no wins…\n"
	.string "I want to go home. Don't tell KIRA.$"

AbandonedShip_Rooms2_1F_Text_DanRematchNotEnoughMons:
	.string "DAN: You need two POKéMON to\n"
	.string "battle us both.$"

AbandonedShip_Rooms2_1F_Text_KiraRematchIntro:
	.string "KIRA: You're still hunting too?\p"
	.string "Loser gives up searching!\n"
	.string "Deal?$"

AbandonedShip_Rooms2_1F_Text_KiraRematchDefeat:
	.string "KIRA: Lost again…\n"
	.string "We're NOT giving up, though.$"

AbandonedShip_Rooms2_1F_Text_KiraPostRematch:
	.string "KIRA: We're staying until we find\n"
	.string "that treasure. Then: a house!$"

AbandonedShip_Rooms2_1F_Text_KiraRematchNotEnoughMons:
	.string "KIRA: Only one POKéMON?\n"
	.string "We need two to battle!$"

AbandonedShip_Rooms2_1F_Text_JaniIntro:
	.string "I can't swim, but I came anyway.\p"
	.string "My roommates say there's DEVON cargo\n"
	.string "still sealed down below!$"

AbandonedShip_Rooms2_1F_Text_JaniDefeat:
	.string "Oops. That went badly.$"

AbandonedShip_Rooms2_1F_Text_JaniPostBattle:
	.string "Walking barefoot in this ship\n"
	.string "is kind of gross.$"

AbandonedShip_Rooms2_1F_Text_GarrisonIntro:
	.string "I rode this ship as a young man,\n"
	.string "when she still carried passengers.\p"
	.string "Now it's just me and the DEVON\n"
	.string "crates. Humor an old man?$"

AbandonedShip_Rooms2_1F_Text_GarrisonDefeat:
	.string "Ah. You sparkle.$"

AbandonedShip_Rooms2_1F_Text_GarrisonPostBattle:
	.string "I saw something sparkle in one\n"
	.string "of the cabins on board.$"
