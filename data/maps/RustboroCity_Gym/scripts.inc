RustboroCity_Gym_MapScripts::
	.byte 0

RustboroCity_Gym_EventScript_Roxanne::
	trainerbattle_single TRAINER_ROXANNE_1, RustboroCity_Gym_Text_RoxanneIntro, RustboroCity_Gym_Text_RoxanneDefeat, RustboroCity_Gym_EventScript_RoxanneDefeated, NO_MUSIC
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_ROCK_TOMB, RustboroCity_Gym_EventScript_GiveRockTomb
	goto RustboroCity_Gym_EventScript_RoxanneRematchOffer

RustboroCity_Gym_EventScript_RoxanneDefeated::
	message RustboroCity_Gym_Text_ReceivedStoneBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox RustboroCity_Gym_Text_StoneBadgeInfoTakeThis, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_RUSTBORO_GYM
	setflag FLAG_BADGE01_GET
	setvar VAR_RUSTBORO_CITY_STATE, 1
	addvar VAR_PETALBURG_GYM_STATE, 1
	setvar VAR_0x8008, 1
	call Common_EventScript_SetGymTrainers
	call_if_eq VAR_PETALBURG_GYM_STATE, 6, Common_EventScript_ReadyPetalburgGymForBattle
	goto RustboroCity_Gym_EventScript_GiveRockTomb
	end

RustboroCity_Gym_EventScript_GiveRockTomb::
	giveitem ITEM_TM_ROCK_TOMB
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_TM_ROCK_TOMB
	msgbox RustboroCity_Gym_Text_ExplainRockTomb, MSGBOX_DEFAULT
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

RustboroCity_Gym_EventScript_RoxanneRematch::
	goto RustboroCity_Gym_EventScript_RoxanneRematchOffer

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
RustboroCity_Gym_EventScript_RoxanneRematchTier3::
	trainerbattle_rematch_double TRAINER_ROXANNE_1, RustboroCity_Gym_Text_RoxannePreRematch3, RustboroCity_Gym_Text_RoxanneRematchDefeat, RustboroCity_Gym_Text_RoxanneRematchNeedTwoMons
	msgbox RustboroCity_Gym_Text_RoxannePostRematch, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_RoxanneRematchTier4::
	trainerbattle_rematch_double TRAINER_ROXANNE_1, RustboroCity_Gym_Text_RoxannePreRematch4, RustboroCity_Gym_Text_RoxanneRematchDefeatElite, RustboroCity_Gym_Text_RoxanneRematchNeedTwoMons
	msgbox RustboroCity_Gym_Text_RoxannePostRematchElite, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_RoxanneRematchTier5::
	trainerbattle_rematch_double TRAINER_ROXANNE_1, RustboroCity_Gym_Text_RoxannePreRematch5, RustboroCity_Gym_Text_RoxanneRematchDefeatElite, RustboroCity_Gym_Text_RoxanneRematchNeedTwoMons
	msgbox RustboroCity_Gym_Text_RoxannePostRematchElite, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_Josh::
	trainerbattle_single TRAINER_JOSH, RustboroCity_Gym_Text_JoshIntro, RustboroCity_Gym_Text_JoshDefeat
	msgbox RustboroCity_Gym_Text_JoshPostBattle, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_Tommy::
	trainerbattle_single TRAINER_TOMMY, RustboroCity_Gym_Text_TommyIntro, RustboroCity_Gym_Text_TommyDefeat
	msgbox RustboroCity_Gym_Text_TommyPostBattle, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_Marc::
	trainerbattle_single TRAINER_MARC, RustboroCity_Gym_Text_MarcIntro, RustboroCity_Gym_Text_MarcDefeat
	msgbox RustboroCity_Gym_Text_MarcPostBattle, MSGBOX_AUTOCLOSE
	end

RustboroCity_Gym_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_RUSTBORO_GYM, RustboroCity_Gym_EventScript_GymGuidePostVictory
	msgbox RustboroCity_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_GymGuidePostVictory::
	msgbox RustboroCity_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE01_GET, RustboroCity_Gym_EventScript_GymStatueCertified
	goto RustboroCity_Gym_EventScript_GymStatue
	end

RustboroCity_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE01_GET, RustboroCity_Gym_EventScript_GymStatueCertified
	goto RustboroCity_Gym_EventScript_GymStatue
	end

RustboroCity_Gym_EventScript_GymStatueCertified::
	msgbox RustboroCity_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

RustboroCity_Gym_EventScript_GymStatue::
	msgbox RustboroCity_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

RustboroCity_Gym_EventScript_RegisterRoxanne::
	lockall
	pokenavcall RustboroCity_Gym_Text_RoxanneRegisterCall
	waitmessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox RustboroCity_Gym_Text_RegisteredRoxanne, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_ROXANNE_MATCH_CALL
	clearflag FLAG_ENABLE_ROXANNE_FIRST_CALL
	releaseall
	end

RustboroCity_Gym_Text_GymGuideAdvice:
	.string "Yo, how's it going?\p"
	.string "Listen, my friend!\n"
	.string "Would you like to become the Champion?\p"
	.string "I'm no Trainer, not me, but I can\n"
	.string "sure give you winning advice.\p"
	.string "That's settled, then! We'll aim for the\n"
	.string "Pokémon Championship together!\p"
	.string "It's your job as a Trainer to collect\n"
	.string "Gym Badges, am I right?\p"
	.string "But Gym Leaders aren't pushovers!\p"
	.string "And that's where I come in!\n"
	.string "I'm here to provide expert advice!\p"
	.string "Roxanne, the Gym Leader, is a user\n"
	.string "of Rock-type Pokémon.\p"
	.string "The Rock type is very durable, but\n"
	.string "it can't stand Water-type and\l"
	.string "Grass-type moves.\p"
	.string "Come see me afterwards, if you beat\n"
	.string "the Gym Leader.\p"
	.string "Well, go for it!$"

RustboroCity_Gym_Text_GymGuidePostVictory:
	.string "Whoa! What a breathtaking victory!\n"
	.string "My cheering must've worked!\p"
	.string "Check your Trainer Card. The Pokémon\n"
	.string "League records every Badge there.\p"
	.string "Gym Leaders are tested and licensed\n"
	.string "by the League. Their Badges count\l"
	.string "anywhere in Hoenn.\p"
	.string "You've climbed the first step on\n"
	.string "the stairs to the Championship!$"

RustboroCity_Gym_Text_JoshIntro:
	.string "Roxanne grades our battles like\n"
	.string "homework.\p"
	.string "I'm going for an A. Don't take Gym\n"
	.string "Trainers lightly!$"

RustboroCity_Gym_Text_JoshDefeat:
	.string "I studied for this…\n"
	.string "I really did!$"

RustboroCity_Gym_Text_JoshPostBattle:
	.string "Devon offered Roxanne a research\n"
	.string "job. Big salary, her own lab.\p"
	.string "She turned it down to keep teaching\n"
	.string "at the Trainer's School. Kids like\l"
	.string "me are why.$"

RustboroCity_Gym_Text_TommyIntro:
	.string "Roxanne makes us write a battle plan\n"
	.string "before every match.\p"
	.string "Mine says I win in four turns!$"

RustboroCity_Gym_Text_TommyDefeat:
	.string "Wow! Roxanne's going to want\n"
	.string "notes on you!$"

RustboroCity_Gym_Text_TommyPostBattle:
	.string "Roxanne grades every battle here.\n"
	.string "Even the ones she isn't in.\p"
	.string "I got a C-minus. From her, that's\n"
	.string "practically a hug.$"

RustboroCity_Gym_Text_MarcIntro:
	.string "Devon wanted this lot for an office.\p"
	.string "Roxanne kept it a Gym. My Rock\n"
	.string "Pokémon thank her!$"

RustboroCity_Gym_Text_MarcDefeat:
	.string "Oh man!\n"
	.string "I cracked first!$"

RustboroCity_Gym_Text_MarcPostBattle:
	.string "Our Leader picked Rock-types at her\n"
	.string "age, against everyone's advice.\p"
	.string "That took nerve. Respect it.$"

RustboroCity_Gym_Text_RoxanneIntro:
	.string "Hello. I am Roxanne, Leader of the\n"
	.string "Rustboro Gym.\p"
	.string "I graduated top of the Trainer's\n"
	.string "School, and I still teach there.\p"
	.string "My students learn from books.\n"
	.string "I learn from challengers.\p"
	.string "Show me what the books left out.$"

RustboroCity_Gym_Text_RoxanneDefeat:
	.string "So… I lost.\p"
	.string "I'll be studying this battle for\n"
	.string "weeks. Thank you.\p"
	.string "League rules say a Trainer who\n"
	.string "defeats a Gym Leader earns this.\p"
	.string "Please accept the Stone Badge.$"

RustboroCity_Gym_Text_ReceivedStoneBadge:
	.string "{PLAYER} received the Stone Badge\n"
	.string "from Roxanne.$"

RustboroCity_Gym_Text_StoneBadgeInfoTakeThis:
	.string "The Stone Badge is yours.\p"
	.string "It grants permission to use Cut\n"
	.string "on small trees in the field.\p"
	.string "I'll give you HM01, too.\n"
	.string "Just face a tree and press A.\p"
	.string "No Pokémon needs to learn Cut\n"
	.string "or even be able to learn it.$"

RustboroCity_Gym_Text_ExplainRockTomb:
	.string "TM39 teaches Rock Tomb.\p"
	.string "The attack damages its target\n"
	.string "and may lower its Speed.\p"
	.string "And TMs are reusable now!\n"
	.string "Teach Rock Tomb as often as\l"
	.string "you like. It won't disappear.$"

RustboroCity_Gym_Text_RoxannePostBattle:
	.string "You adapted to my strategy faster\n"
	.string "than I could revise it.\p"
	.string "Challenge the other Leaders. Each one\n"
	.string "will teach you something different.$"

RustboroCity_Gym_Text_GymStatue:
	.string "Rustboro City Pokémon Gym$"

RustboroCity_Gym_Text_GymStatueCertified:
	.string "Rustboro City Pokémon Gym\p"
	.string "Roxanne's Certified Trainers:\n"
	.string "{PLAYER}$"

RustboroCity_Gym_Text_RoxanneRegisterCall:
	.string "… … … … … …\n"
	.string "… … … … … Beep!\p"
	.string "Roxanne: Hello, {PLAYER}? This is\n"
	.string "Roxanne of Rustboro.\p"
	.string "Brawly tells me you're climbing fast.\n"
	.string "I've added you to my lesson plans.\p"
	.string "I'll rebuild my Gym so you can\n"
	.string "challenge us again. Study hard.\p"
	.string "… … … … … …\n"
	.string "… … … … … Click!$"

RustboroCity_Gym_Text_RegisteredRoxanne:
	.string "Registered Gym Leader Roxanne\n"
	.string "in the PokéNav.$"

RustboroCity_Gym_Text_RoxannePreRematch:
	.string "Roxanne: Welcome back.\p"
	.string "I've reviewed our last battle eleven\n"
	.string "times. I found four mistakes.\p"
	.string "All mine. Let us see if I fixed them!$"

RustboroCity_Gym_Text_RoxanneRematchDefeat:
	.string "…Five mistakes, then.$"

RustboroCity_Gym_Text_RoxannePostRematch:
	.string "Roxanne: Every loss is a lesson.\n"
	.string "You are a very good teacher.\p"
	.string "Please don't tell my students.$"

RustboroCity_Gym_Text_RoxanneRematchNeedTwoMons:
	.string "Roxanne: Welcome back.\p"
	.string "Oh dear. You only have one Pokémon\n"
	.string "with you?\p"
	.string "Rematches here are two at a time.\n"
	.string "Please return with one more.$"

RustboroCity_Gym_Text_RoxannePreRematch3:
	.string "Roxanne: I rebuilt my team from the\n"
	.string "textbook up.\p"
	.string "Then I threw the textbook away.\p"
	.string "Let's see which version of me wins.$"

RustboroCity_Gym_Text_RoxannePreRematch4:
	.string "Roxanne: The League asked me to join\n"
	.string "its Gym review board.\p"
	.string "I said yes, on one condition: I keep\n"
	.string "battling you until I stop losing.\p"
	.string "Show me where I'm still wrong.$"

RustboroCity_Gym_Text_RoxannePreRematch5:
	.string "Roxanne: This is the strongest team\n"
	.string "I can build. No theory left. Only\l"
	.string "what's true.\p"
	.string "Teach me one more lesson, Champion.$"

RustboroCity_Gym_Text_RoxanneRematchDefeatElite:
	.string "…Fascinating. I'll need to write\n"
	.string "a new chapter.$"

RustboroCity_Gym_Text_RoxannePostRematchElite:
	.string "Roxanne: Wally came by asking to\n"
	.string "read my notes on you.\p"
	.string "I said no. Some lessons have to be\n"
	.string "earned in battle.$"

@ Wally's Journey: growing Gym Leader rematches (see WallyJourney_ScaleLeaderParty).
RustboroCity_Gym_EventScript_RoxanneRematchOffer::
	setvar VAR_0x8004, 1
	special WallyJourney_SetupLeaderRematch
	switch VAR_RESULT
	case 1, RustboroCity_Gym_EventScript_RoxanneRematchStage1
	case 2, RustboroCity_Gym_EventScript_RoxanneRematchStage2
	case 3, RustboroCity_Gym_EventScript_RoxanneRematchStage3
	case 4, RustboroCity_Gym_EventScript_RoxanneRematchStage4
	goto RustboroCity_Gym_EventScript_RoxanneRematchStage1

RustboroCity_Gym_EventScript_RoxanneRematchStage1::
	msgbox RustboroCity_Gym_Text_RoxanneGrowOffer1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_Gym_EventScript_RoxanneRematchDecline
	cleartrainerflag TRAINER_ROXANNE_2
	trainerbattle_no_intro TRAINER_ROXANNE_2, RustboroCity_Gym_Text_RoxanneGrowDefeat1
	msgbox RustboroCity_Gym_Text_RoxanneGrowAfter1, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_RoxanneRematchStage2::
	msgbox RustboroCity_Gym_Text_RoxanneGrowOffer2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_Gym_EventScript_RoxanneRematchDecline
	cleartrainerflag TRAINER_ROXANNE_2
	trainerbattle_no_intro TRAINER_ROXANNE_2, RustboroCity_Gym_Text_RoxanneGrowDefeat2
	msgbox RustboroCity_Gym_Text_RoxanneGrowAfter2, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_RoxanneRematchStage3::
	msgbox RustboroCity_Gym_Text_RoxanneGrowOffer3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_Gym_EventScript_RoxanneRematchDecline
	cleartrainerflag TRAINER_ROXANNE_3
	trainerbattle_double TRAINER_ROXANNE_3, RustboroCity_Gym_Text_RoxanneGrowGo, RustboroCity_Gym_Text_RoxanneGrowDefeat3, RustboroCity_Gym_Text_RoxanneGrowNeedTwo
	setvar VAR_0x8004, 1
	special WallyJourney_RecordLeaderPostgameWin
	msgbox RustboroCity_Gym_Text_RoxanneGrowAfter3, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_RoxanneRematchStage4::
	msgbox RustboroCity_Gym_Text_RoxanneGrowOffer4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_Gym_EventScript_RoxanneRematchDecline
	cleartrainerflag TRAINER_ROXANNE_4
	trainerbattle_double TRAINER_ROXANNE_4, RustboroCity_Gym_Text_RoxanneGrowGo, RustboroCity_Gym_Text_RoxanneGrowDefeat4, RustboroCity_Gym_Text_RoxanneGrowNeedTwo
	msgbox RustboroCity_Gym_Text_RoxanneGrowAfter4, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_EventScript_RoxanneRematchDecline::
	msgbox RustboroCity_Gym_Text_RoxannePostBattle, MSGBOX_DEFAULT
	release
	end

RustboroCity_Gym_Text_RoxanneGrowGo:
	.string "Here we go!$"

RustboroCity_Gym_Text_RoxanneGrowNeedTwo:
	.string "You need two Pokémon for this\n"
	.string "battle. Come back with more.$"

RustboroCity_Gym_Text_RoxanneGrowOffer1:
	.string "Roxanne: You have {STR_VAR_1} Badges now.\p"
	.string "A Leader chooses a team to suit a\n"
	.string "challenger's Badges.\p"
	.string "You've earned a harder exam.\n"
	.string "Will you sit it?$"

RustboroCity_Gym_Text_RoxanneGrowDefeat1:
	.string "My notes were right about you.\n"
	.string "You're ahead of the lesson plan.$"

RustboroCity_Gym_Text_RoxanneGrowAfter1:
	.string "Come back after your next Badge.\n"
	.string "I'll have revised my team again.$"

RustboroCity_Gym_Text_RoxanneGrowOffer2:
	.string "Roxanne: Eight Badges. Next is the\n"
	.string "Pokémon League.\p"
	.string "So I'll hold back much less this\n"
	.string "time. Shall we test you?$"

RustboroCity_Gym_Text_RoxanneGrowDefeat2:
	.string "You're ready. I'd stake my\n"
	.string "teaching certificate on it.$"

RustboroCity_Gym_Text_RoxanneGrowAfter2:
	.string "Go to Ever Grande City.\n"
	.string "I'll tell my students about you.$"

RustboroCity_Gym_Text_RoxanneGrowOffer3:
	.string "Roxanne: Our Champion came back to\n"
	.string "my Gym. That's a lesson in itself.\p"
	.string "My full team is ready.\n"
	.string "Will you battle me?$"

RustboroCity_Gym_Text_RoxanneGrowDefeat3:
	.string "Even my best research team\n"
	.string "couldn't hold you.$"

RustboroCity_Gym_Text_RoxanneGrowAfter3:
	.string "Defend your title, Champion.\n"
	.string "I'll keep refining my team.$"

RustboroCity_Gym_Text_RoxanneGrowOffer4:
	.string "Roxanne: You've defended your title.\p"
	.string "This is the strongest team I can\n"
	.string "build. Will you face it?$"

RustboroCity_Gym_Text_RoxanneGrowDefeat4:
	.string "That was my best work.\n"
	.string "You earned every point of it.$"

RustboroCity_Gym_Text_RoxanneGrowAfter4:
	.string "Any time you want a rematch,\n"
	.string "my classroom is open.$"

