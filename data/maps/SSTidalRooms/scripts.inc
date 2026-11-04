SSTidalRooms_MapScripts::
	.byte 0

SSTidalRooms_EventScript_SnatchGiver::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_TM_SNATCH, SSTidalRooms_EventScript_ExplainSnatch
	msgbox SSTidalRooms_Text_NotSuspiciousTakeThis, MSGBOX_DEFAULT
	giveitem ITEM_TM_SNATCH
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_TM_SNATCH
	msgbox SSTidalRooms_Text_ExplainSnatch, MSGBOX_DEFAULT
	release
	end

SSTidalRooms_EventScript_ExplainSnatch::
	msgbox SSTidalRooms_Text_ExplainSnatch, MSGBOX_DEFAULT
	release
	end

SSTidalRooms_EventScript_Bed::
	lockall
	msgbox SSTidalRooms_Text_TakeRestOnBed, MSGBOX_DEFAULT
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	call SSTidalRooms_EventScript_ProgessCruiseAfterBed
	releaseall
	end

SSTidalRooms_EventScript_Colton::
	trainerbattle_single TRAINER_COLTON, SSTidalRooms_Text_ColtonIntro, SSTidalRooms_Text_ColtonDefeat
	msgbox SSTidalRooms_Text_ColtonPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Micah::
	trainerbattle_single TRAINER_MICAH, SSTidalRooms_Text_MicahIntro, SSTidalRooms_Text_MicahDefeat
	msgbox SSTidalRooms_Text_MicahPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Thomas::
	trainerbattle_single TRAINER_THOMAS, SSTidalRooms_Text_ThomasIntro, SSTidalRooms_Text_ThomasDefeat
	msgbox SSTidalRooms_Text_ThomasPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Jed::
	trainerbattle_double TRAINER_LEA_AND_JED, SSTidalRooms_Text_JedIntro, SSTidalRooms_Text_JedDefeat, SSTidalRooms_Text_JedNotEnoughMons
	msgbox SSTidalRooms_Text_JedPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Lea::
	trainerbattle_double TRAINER_LEA_AND_JED, SSTidalRooms_Text_LeaIntro, SSTidalRooms_Text_LeaDefeat, SSTidalRooms_Text_LeaNotEnoughMons
	msgbox SSTidalRooms_Text_LeaPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Garret::
	trainerbattle_single TRAINER_GARRET, SSTidalRooms_Text_GarretIntro, SSTidalRooms_Text_GarretDefeat
	msgbox SSTidalRooms_Text_GarretPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_EventScript_Naomi::
	trainerbattle_single TRAINER_NAOMI, SSTidalRooms_Text_NaomiIntro, SSTidalRooms_Text_NaomiDefeat
	msgbox SSTidalRooms_Text_NaomiPostBattle, MSGBOX_AUTOCLOSE
	end

SSTidalRooms_Text_TakeRestOnBed:
	.string "There's a bed…\n"
	.string "Let's take a rest.$"

SSTidalRooms_Text_ColtonIntro:
	.string "I sail to Lilycove for Contests!\p"
	.string "Wallace got his start in those halls.\n"
	.string "This ferry's all I can afford!$"

SSTidalRooms_Text_ColtonDefeat:
	.string "What a delightful match!$"

SSTidalRooms_Text_ColtonPostBattle:
	.string "I'm so excited for the next\n"
	.string "Contest! What will I see?$"

SSTidalRooms_Text_MicahIntro:
	.string "Are your friends strong?\p"
	.string "I sailed beside Glacia's ship when she\n"
	.string "first came to Hoenn. Cold hands!$"

SSTidalRooms_Text_MicahDefeat:
	.string "Your friends are very strong.$"

SSTidalRooms_Text_MicahPostBattle:
	.string "Friends needn't be human.\n"
	.string "My Pokémon are my dearest friends.$"

SSTidalRooms_Text_ThomasIntro:
	.string "Child… did you knock?\p"
	.string "Mr. Stone of Devon rides this ferry.\n"
	.string "Manners matter here.$"

SSTidalRooms_Text_ThomasDefeat:
	.string "A loss is accepted calmly.$"

SSTidalRooms_Text_ThomasPostBattle:
	.string "A Gentleman is never ruffled.\n"
	.string "That is the code.$"

SSTidalRooms_Text_JedIntro:
	.string "Jed: It's our honeymoon!\p"
	.string "We're going to see Southern Island!\n"
	.string "Lovey-dovey power!$"

SSTidalRooms_Text_JedDefeat:
	.string "Jed: Sigh… Not on our\n"
	.string "honeymoon…$"

SSTidalRooms_Text_JedPostBattle:
	.string "Jed: First time our love lost.\n"
	.string "You're amazing!$"

SSTidalRooms_Text_JedNotEnoughMons:
	.string "Jed: Only one Pokémon? We battle\n"
	.string "as a couple. Bring two!$"

SSTidalRooms_Text_LeaIntro:
	.string "Lea: Honeymoon! They say twin dragons\n"
	.string "bless couples on Southern Island!$"

SSTidalRooms_Text_LeaDefeat:
	.string "Lea: Oh, boo! Our love\n"
	.string "deserved better!$"

SSTidalRooms_Text_LeaPostBattle:
	.string "Lea: Our love lost?! You must be\n"
	.string "an awesome Trainer!$"

SSTidalRooms_Text_LeaNotEnoughMons:
	.string "Lea: We wanted to battle! But you\n"
	.string "need two Pokémon…$"

SSTidalRooms_Text_GarretIntro:
	.string "I'm bored.\p"
	.string "Father owns twelve houses in\n"
	.string "Lilycove and none feel like home.\l"
	.string "Entertain me.$"

SSTidalRooms_Text_GarretDefeat:
	.string "…That will do.\n"
	.string "Father will hear of this.$"

SSTidalRooms_Text_GarretPostBattle:
	.string "Perhaps Father will buy me a yacht.\p"
	.string "He says Hoenn has no room left for\n"
	.string "houses, so he buys boats instead.$"

SSTidalRooms_Text_NaomiIntro:
	.string "What an adorable Trainer!\p"
	.string "I'm touring Hoenn's legend sites by\n"
	.string "ferry. Tea? Or a battle?$"

SSTidalRooms_Text_NaomiDefeat:
	.string "I see. The active type.$"

SSTidalRooms_Text_NaomiPostBattle:
	.string "A world cruise has its charms…\p"
	.string "But touring Hoenn by ferry is\n"
	.string "appealing in its own way.$"

SSTidalRooms_Text_NotSuspiciousTakeThis:
	.string "Uh… Hi! I… I'm not acting suspicious!\n"
	.string "Uh… You can have this! For free!\p"
	.string "It… Honestly, I didn't Snatch it from\n"
	.string "someone! I'd never do such a thing!\l"
	.string "It's clean! You can use it!$"

SSTidalRooms_Text_ExplainSnatch:
	.string "Snatch steals the beneficial effects\n"
	.string "of certain moves before they can be\l"
	.string "used by a foe or ally.$"

