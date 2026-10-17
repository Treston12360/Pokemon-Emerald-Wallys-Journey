VerdanturfTown_Mart_MapScripts::
	.byte 0

VerdanturfTown_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart VerdanturfTown_Mart_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
VerdanturfTown_Mart_Pokemart:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_BURN_HEAL
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_X_SP_ATK
	.2byte ITEM_FLUFFY_TAIL
	.2byte ITEM_UPGRADE
	.2byte ITEM_ELECTIRIZER
	.2byte ITEM_MAGMARIZER
	pokemartlistend

VerdanturfTown_Mart_EventScript_Boy::
	msgbox VerdanturfTown_Mart_Text_XSpecialIsCrucial, MSGBOX_NPC
	end

VerdanturfTown_Mart_EventScript_ExpertF::
	msgbox VerdanturfTown_Mart_Text_NoStrategyGuidesForBattleTent, MSGBOX_NPC
	end

VerdanturfTown_Mart_EventScript_Lass::
	msgbox VerdanturfTown_Mart_Text_NestBallOnWeakenedPokemon, MSGBOX_NPC
	end

VerdanturfTown_Mart_Text_XSpecialIsCrucial:
	.string "X SPECIAL boosts some moves for one\n"
	.string "battle.\p"
	.string "Good to have. Better to not need.$"

VerdanturfTown_Mart_Text_NoStrategyGuidesForBattleTent:
	.string "No strategy guides for the BATTLE\n"
	.string "TENT anywhere.\p"
	.string "Guess you have to rely on your own\n"
	.string "wits. Like everything else.$"

VerdanturfTown_Mart_Text_NestBallOnWeakenedPokemon:
	.string "A sleeping or paralyzed POKéMON is\n"
	.string "easier to catch.\p"
	.string "With only plain POKé BALLS, that's\n"
	.string "the whole game.$"

