PetalburgCity_House2_MapScripts::
	.byte 0

PetalburgCity_House2_EventScript_Woman::
	goto_if_set FLAG_SYS_GAME_CLEAR, PetalburgCity_House2_Text_NormanBecameGymLeader_W0_Script
	goto_if_set FLAG_DEFEATED_PETALBURG_GYM, PetalburgCity_House2_Text_NormanBecameGymLeader_W1_Script
	msgbox PetalburgCity_House2_Text_NormanBecameGymLeader, MSGBOX_NPC
	end

PetalburgCity_House2_Text_NormanBecameGymLeader_W0_Script:
	msgbox PetalburgCity_House2_Text_NormanBecameGymLeader_W0, MSGBOX_NPC
	end

PetalburgCity_House2_Text_NormanBecameGymLeader_W1_Script:
	msgbox PetalburgCity_House2_Text_NormanBecameGymLeader_W1, MSGBOX_NPC
	end

PetalburgCity_House2_EventScript_SchoolKid::
	goto_if_set FLAG_SYS_GAME_CLEAR, PetalburgCity_House2_EventScript_SchoolKidAfterTitle
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, PetalburgCity_House2_Text_BattledNormanOnce_W0_Script
	msgbox PetalburgCity_House2_Text_BattledNormanOnce, MSGBOX_NPC
	end

PetalburgCity_House2_Text_BattledNormanOnce_W0_Script:
	msgbox PetalburgCity_House2_Text_BattledNormanOnce_W0, MSGBOX_NPC
	end

PetalburgCity_House2_Text_NormanBecameGymLeader:
	.string "Norman came all the way from Johto\n"
	.string "to take over our Gym.\p"
	.string "Worked day and night. His family\n"
	.string "followed once he could afford a\l"
	.string "house in Littleroot.\p"
	.string "Driven man. Maybe too driven.$"

PetalburgCity_House2_Text_BattledNormanOnce:
	.string "I battled Norman once. He was way\n"
	.string "too strong.\p"
	.string "But afterward he asked about my\n"
	.string "kids. Then he went quiet.\p"
	.string "I think he misses his own.$"

PetalburgCity_House2_Text_NormanBecameGymLeader_W0:
	.string "Norman's kid is the Champion now.\p"
	.string "He hung a photo on the Gym wall.\n"
	.string "Won't let anyone touch it.$"

PetalburgCity_House2_Text_NormanBecameGymLeader_W1:
	.string "Word is Norman's own kid beat him.\p"
	.string "He left the Gym before dark that\n"
	.string "night. First time anyone can\l"
	.string "remember.$"

PetalburgCity_House2_Text_BattledNormanOnce_W0:
	.string "That Wally kid lived three doors\n"
	.string "down. Norman lent him a Zigzagoon\l"
	.string "to catch his first Pokémon.\p"
	.string "Now he's the Champion.\n"
	.string "This town, huh?$"

PetalburgCity_House2_EventScript_SchoolKidAfterTitle::
	msgbox PetalburgCity_House2_Text_SchoolKidAfterTitle, MSGBOX_NPC
	end

PetalburgCity_House2_Text_SchoolKidAfterTitle:
	.string "That Wally kid lived three doors down.\n"
	.string "He was Champion for a while.\p"
	.string "Then Norman's kid beat him!\n"
	.string "This town, huh?$"
