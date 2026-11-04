MtPyre_1F_MapScripts::
	.byte 0

MtPyre_1F_EventScript_CleanseTagWoman::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_CLEANSE_TAG, MtPyre_1F_EventScript_ReceivedCleanseTag
	msgbox MtPyre_1F_Text_TakeThisForYourOwnGood, MSGBOX_DEFAULT
	giveitem ITEM_CLEANSE_TAG
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_CLEANSE_TAG
	release
	end

MtPyre_1F_EventScript_ReceivedCleanseTag::
	msgbox MtPyre_1F_Text_ExplainCleanseTag, MSGBOX_DEFAULT
	release
	end

MtPyre_1F_EventScript_PokefanF::
	msgbox MtPyre_1F_Text_ComeToPayRespects, MSGBOX_NPC
	end

MtPyre_1F_EventScript_Man::
	msgbox MtPyre_1F_Text_RestingPlaceOfZigzagoon, MSGBOX_NPC
	end

MtPyre_1F_Text_TakeThisForYourOwnGood:
	.string "All sorts of beings wander the slopes\n"
	.string "of Mt. Pyre…\p"
	.string "There is no telling what may happen.\n"
	.string "Take this. It's for your own good.$"

MtPyre_1F_Text_ExplainCleanseTag:
	.string "Have a Pokémon hold that\n"
	.string "Cleanse Tag.\p"
	.string "It will help ward off wild Pokémon.$"

MtPyre_1F_Text_ComeToPayRespects:
	.string "Did you come to remember\n"
	.string "a Pokémon you loved?\p"
	.string "People travel here from all\n"
	.string "over Hoenn to do that.\p"
	.string "Some come to grieve. Others\n"
	.string "come to say thank you.\l"
	.string "Both are welcome.$"

MtPyre_1F_Text_RestingPlaceOfZigzagoon:
	.string "My Zigzagoon used to bring me\n"
	.string "anything it found on the road.\p"
	.string "A pebble. A twig. Once, half\n"
	.string "a sandwich.\p"
	.string "I kept the pebble.\n"
	.string "I don't know why I chose that one.$"

