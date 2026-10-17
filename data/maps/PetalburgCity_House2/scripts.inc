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
	goto_if_set FLAG_SYS_GAME_CLEAR, PetalburgCity_House2_EventScript_ChildAfterPlayerChampion
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, PetalburgCity_House2_Text_BattledNormanOnce_W0_Script
	msgbox PetalburgCity_House2_Text_BattledNormanOnce, MSGBOX_NPC
	end

PetalburgCity_House2_EventScript_ChildAfterPlayerChampion:
	msgbox PetalburgCity_House2_Text_BattledNormanOnce_AfterPlayerChampion, MSGBOX_NPC
	end

PetalburgCity_House2_Text_BattledNormanOnce_W0_Script:
	msgbox PetalburgCity_House2_Text_BattledNormanOnce_W0, MSGBOX_NPC
	end

PetalburgCity_House2_Text_NormanBecameGymLeader:
	.string "NORMAN came all the way from JOHTO\n"
	.string "to take over our GYM.\p"
	.string "Worked day and night. His family\n"
	.string "followed once he could afford a\l"
	.string "house in LITTLEROOT.\p"
	.string "Driven man. Maybe too driven.$"

PetalburgCity_House2_Text_BattledNormanOnce:
	.string "I battled NORMAN once. He was way\n"
	.string "too strong.\p"
	.string "But afterward he asked about my\n"
	.string "kids. Then he went quiet.\p"
	.string "I think he misses his own.$"

PetalburgCity_House2_Text_NormanBecameGymLeader_W0:
	.string "NORMAN's kid is the CHAMPION now.\p"
	.string "He hung a photo on the GYM wall.\n"
	.string "Won't let anyone touch it.$"

PetalburgCity_House2_Text_NormanBecameGymLeader_W1:
	.string "Word is NORMAN's own kid beat him.\p"
	.string "He left the GYM before dark that\n"
	.string "night. First time anyone can\l"
	.string "remember.$"

PetalburgCity_House2_Text_BattledNormanOnce_W0:
	.string "That WALLY kid lived three doors\n"
	.string "down. NORMAN lent him a ZIGZAGOON\l"
	.string "to catch his first POKéMON.\p"
	.string "Now he's the CHAMPION.\n"
	.string "This town, huh?$"

PetalburgCity_House2_Text_BattledNormanOnce_AfterPlayerChampion:
	.string "That WALLY kid used to live\n"
	.string "three doors down.\p"
	.string "NORMAN lent him the POKéMON\n"
	.string "that helped him catch RALTS.\p"
	.string "Now WALLY and you have both\n"
	.string "held the CHAMPION title.\p"
	.string "PETALBURG will be talking\n"
	.string "about that for years.$"
