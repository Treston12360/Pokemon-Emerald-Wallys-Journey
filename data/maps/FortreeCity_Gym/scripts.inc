FortreeCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FortreeCity_Gym_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, FortreeCity_Gym_OnWarp
	.byte 0

FortreeCity_Gym_OnTransition:
	special RotatingGate_InitPuzzle
	setflag FLAG_TEMP_HIDE_FOLLOWER @ Hide follower bc of rotating gates
	end

FortreeCity_Gym_OnWarp:
	map_script_2 VAR_TEMP_0, VAR_TEMP_0, FortreeCity_Gym_EventScript_InitRotatingGates
	.2byte 0

@ NOTE: This rotating gate puzzle makes use of VAR_TEMP_0 - VAR_TEMP_3
FortreeCity_Gym_EventScript_InitRotatingGates::
	special RotatingGate_InitPuzzleAndGraphics
	end

FortreeCity_Gym_EventScript_Winona::
	trainerbattle_single TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_AERIAL_ACE, FortreeCity_Gym_EventScript_GiveAerialAce2
	goto FortreeCity_Gym_EventScript_WinonaRematchOffer

FortreeCity_Gym_EventScript_WinonaDefeated::
	message FortreeCity_Gym_Text_ReceivedFeatherBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox FortreeCity_Gym_Text_ExplainFeatherBadgeTakeThis, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_FORTREE_GYM
	setflag FLAG_BADGE06_GET
	setvar VAR_WALLY_RIVAL_PENDING, 6
	setvar VAR_0x8008, 6
	call Common_EventScript_SetGymTrainers
	call FortreeCity_Gym_EventScript_GiveAerialAce
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox FortreeCity_Gym_Text_RegisteredWinona, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_WINONA_MATCH_CALL
	setvar VAR_SCOTT_FORTREE_CALL_STEP_COUNTER, 0
	setflag FLAG_SCOTT_CALL_FORTREE_GYM
	release
	end

FortreeCity_Gym_EventScript_GiveAerialAce2::
	giveitem ITEM_TM_AERIAL_ACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox FortreeCity_Gym_Text_ExplainAerialAce, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_AERIAL_ACE
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

FortreeCity_Gym_EventScript_GiveAerialAce::
	giveitem ITEM_TM_AERIAL_ACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_BagIsFull
	msgbox FortreeCity_Gym_Text_ExplainAerialAce, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_AERIAL_ACE
	call WallyJourney_EventScript_GiveGymHMs
	return

FortreeCity_Gym_EventScript_WinonaRematch::
	goto FortreeCity_Gym_EventScript_WinonaRematchOffer

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
FortreeCity_Gym_EventScript_WinonaRematchTier3::
	trainerbattle_rematch_double TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaPreRematch3, FortreeCity_Gym_Text_WinonaRematchDefeat, FortreeCity_Gym_Text_WinonaRematchNeedTwoMons
	msgbox FortreeCity_Gym_Text_WinonaPostRematch, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_WinonaRematchTier4::
	trainerbattle_rematch_double TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaPreRematch4, FortreeCity_Gym_Text_WinonaRematchDefeatElite, FortreeCity_Gym_Text_WinonaRematchNeedTwoMons
	msgbox FortreeCity_Gym_Text_WinonaPostRematchElite, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_WinonaRematchTier5::
	trainerbattle_rematch_double TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaPreRematch5, FortreeCity_Gym_Text_WinonaRematchDefeatElite, FortreeCity_Gym_Text_WinonaRematchNeedTwoMons
	msgbox FortreeCity_Gym_Text_WinonaPostRematchElite, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Jared::
	trainerbattle_single TRAINER_JARED, FortreeCity_Gym_Text_JaredIntro, FortreeCity_Gym_Text_JaredDefeat
	msgbox FortreeCity_Gym_Text_JaredPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Edwardo::
	trainerbattle_single TRAINER_EDWARDO, FortreeCity_Gym_Text_EdwardoIntro, FortreeCity_Gym_Text_EdwardoDefeat
	msgbox FortreeCity_Gym_Text_EdwardoPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Flint::
	trainerbattle_single TRAINER_FLINT, FortreeCity_Gym_Text_FlintIntro, FortreeCity_Gym_Text_FlintDefeat
	msgbox FortreeCity_Gym_Text_FlintPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Ashley::
	trainerbattle_single TRAINER_ASHLEY, FortreeCity_Gym_Text_AshleyIntro, FortreeCity_Gym_Text_AshleyDefeat
	msgbox FortreeCity_Gym_Text_AshleyPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Humberto::
	trainerbattle_single TRAINER_HUMBERTO, FortreeCity_Gym_Text_HumbertoIntro, FortreeCity_Gym_Text_HumbertoDefeat
	msgbox FortreeCity_Gym_Text_HumbertoPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Darius::
	trainerbattle_single TRAINER_DARIUS, FortreeCity_Gym_Text_DariusIntro, FortreeCity_Gym_Text_DariusDefeat
	msgbox FortreeCity_Gym_Text_DariusPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_FORTREE_GYM, FortreeCity_Gym_EventScript_GymGuidePostVictory
	msgbox FortreeCity_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_GymGuidePostVictory::
	msgbox FortreeCity_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE06_GET, FortreeCity_Gym_EventScript_GymStatueCertified
	goto FortreeCity_Gym_EventScript_GymStatue
	end

FortreeCity_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE06_GET, FortreeCity_Gym_EventScript_GymStatueCertified
	goto FortreeCity_Gym_EventScript_GymStatue
	end

FortreeCity_Gym_EventScript_GymStatueCertified::
	msgbox FortreeCity_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

FortreeCity_Gym_EventScript_GymStatue::
	msgbox FortreeCity_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

FortreeCity_Gym_Text_GymGuideAdvice:
	.string "Yo, how's it going, Champion-\n"
	.string "bound {PLAYER}?\p"
	.string "Fortree Gym Leader Winona is\n"
	.string "a master of Flying-type Pokémon.\p"
	.string "She's waiting at the back of this Gym,\n"
	.string "behind the rotating doors.\p"
	.string "She's waiting for new challengers\n"
	.string "who are trying to take wing!\p"
	.string "Okay, go for it!$"

FortreeCity_Gym_Text_GymGuidePostVictory:
	.string "You did it!\n"
	.string "You've achieved liftoff!\p"
	.string "Heads up: the League counts Badges,\n"
	.string "not who you beat. Eight Badges is\l"
	.string "the only ticket to Victory Road.$"

FortreeCity_Gym_Text_JaredIntro:
	.string "My family has lived in the same\n"
	.string "treehouse for five generations!\p"
	.string "We don't move. And I don't lose!$"

FortreeCity_Gym_Text_JaredDefeat:
	.string "You…\n"
	.string "You fly higher than me.$"

FortreeCity_Gym_Text_JaredPostBattle:
	.string "Fortree can't build outward. The\n"
	.string "forest won't allow it.\p"
	.string "Winona says that's a gift, not a\n"
	.string "problem. Not everyone agrees.$"

FortreeCity_Gym_Text_EdwardoIntro:
	.string "Every kid in Fortree wants to join\n"
	.string "this Gym.\p"
	.string "Winona only takes ten a year.\n"
	.string "I'm one of them! I can't lose!$"

FortreeCity_Gym_Text_EdwardoDefeat:
	.string "Too heavy a load for my\n"
	.string "wings…$"

FortreeCity_Gym_Text_EdwardoPostBattle:
	.string "Winona keeps a nest room above the\n"
	.string "Gym.\p"
	.string "Every injured bird Pokémon in the\n"
	.string "forest ends up there. She feeds\l"
	.string "them herself.$"

FortreeCity_Gym_Text_FlintIntro:
	.string "Winona saved my Swablu from a storm\n"
	.string "when I was little.\p"
	.string "I owe her. You won't reach her!$"

FortreeCity_Gym_Text_FlintDefeat:
	.string "Winona… I'm sorry…\n"
	.string "I got grounded.$"

FortreeCity_Gym_Text_FlintPostBattle:
	.string "I joined because Winona saved my\n"
	.string "Swablu from a storm.\p"
	.string "…And maybe because she's amazing.\n"
	.string "Don't tell her. …Blush.$"

FortreeCity_Gym_Text_AshleyIntro:
	.string "Winona found me in the nest room,\n"
	.string "feeding a hurt Taillow.\p"
	.string "She made me a Trainer that day!$"

FortreeCity_Gym_Text_AshleyDefeat:
	.string "I was beaten…\n"
	.string "Winona will still be kind about it.$"

FortreeCity_Gym_Text_AshleyPostBattle:
	.string "Thanks to Winona, Fortree lives\n"
	.string "without fear.\p"
	.string "She flies patrols over the forest\n"
	.string "every dawn.$"

FortreeCity_Gym_Text_HumbertoIntro:
	.string "Winona's face in battle shines with\n"
	.string "determination… I won't let you see\l"
	.string "it!$"

FortreeCity_Gym_Text_HumbertoDefeat:
	.string "Urk!\n"
	.string "You flew right past my guard.$"

FortreeCity_Gym_Text_HumbertoPostBattle:
	.string "Before Winona, this Gym was all\n"
	.string "show-offs and stunts.\p"
	.string "She made it about the birds again.$"

FortreeCity_Gym_Text_DariusIntro:
	.string "Developers want to clear the forest\n"
	.string "floor for houses.\p"
	.string "Winona's against it. So am I!$"

FortreeCity_Gym_Text_DariusDefeat:
	.string "You know your Flying types!$"

FortreeCity_Gym_Text_DariusPostBattle:
	.string "Winona's Pokémon don't show off.\p"
	.string "They fly like they're protecting\n"
	.string "something. Because they are.$"

FortreeCity_Gym_Text_WinonaIntro:
	.string "I am Winona, Leader of the Fortree\n"
	.string "Gym.\p"
	.string "I grew up in these treetops, with\n"
	.string "nothing above me but sky.\p"
	.string "The sky belongs to no one. My\n"
	.string "Pokémon and I only borrow it.\p"
	.string "Witness how gracefully we share it!$"

FortreeCity_Gym_Text_WinonaDefeat:
	.string "You flew past me as if the wind\n"
	.string "were yours…\p"
	.string "This Gym Badge is yours too.\n"
	.string "You've earned it.$"

FortreeCity_Gym_Text_ReceivedFeatherBadge:
	.string "{PLAYER} received the Feather Badge\n"
	.string "from Winona.$"

FortreeCity_Gym_Text_ExplainFeatherBadgeTakeThis:
	.string "With the Feather Badge, all Pokémon up\n"
	.string "to LV 70, even those received through\l"
	.string "trades, will obey your every command.\p"
	.string "You will also be able to use\n"
	.string "the HM move Fly outside of battle.\p"
	.string "And this…\n"
	.string "This is a gift from me.$"

FortreeCity_Gym_Text_ExplainAerialAce:
	.string "TM40 contains Aerial Ace.\p"
	.string "Its speed…\n"
	.string "No Pokémon should be able to avoid it.\p"
	.string "… … … … … …$"

FortreeCity_Gym_Text_RegisteredWinona:
	.string "Registered Gym Leader Winona\n"
	.string "in the PokéNav.$"

FortreeCity_Gym_Text_WinonaPostBattle:
	.string "Though I fell to you, I will remain\n"
	.string "devoted to Bird Pokémon.\p"
	.string "A boy named Wally came through a week\n"
	.string "ago. He read my whole strategy by the\l"
	.string "second turn.\p"
	.string "Polite boy. A little too sure of\n"
	.string "himself. Terrifying Trainer.$"

FortreeCity_Gym_Text_GymStatue:
	.string "Fortree City Pokémon Gym$"

FortreeCity_Gym_Text_GymStatueCertified:
	.string "Fortree City Pokémon Gym\p"
	.string "Winona's Certified Trainers:\n"
	.string "{PLAYER}$"

FortreeCity_Gym_Text_WinonaPreRematch:
	.string "Winona: We can never escape\n"
	.string "gravity.\p"
	.string "But we can choose how high we climb.\n"
	.string "You've climbed to the very top.\p"
	.string "Show me your power at full flight!$"

FortreeCity_Gym_Text_WinonaRematchDefeat:
	.string "You climbed higher again…\n"
	.string "I couldn't follow.$"

FortreeCity_Gym_Text_WinonaPostRematch:
	.string "Winona: My wings are bruised, not\n"
	.string "broken.\p"
	.string "I'll rise again. The birds taught me\n"
	.string "that.$"

FortreeCity_Gym_Text_WinonaRematchNeedTwoMons:
	.string "Winona: We can never escape\n"
	.string "gravity.\p"
	.string "Oh… You have only one Pokémon\n"
	.string "with you?\p"
	.string "Rematches here are two at a time.\n"
	.string "Please return with two.$"

FortreeCity_Gym_Text_WinonaPreRematch3:
	.string "Winona: Since the storms, my Bird\n"
	.string "Pokémon have flown rescue patrols\l"
	.string "every dawn.\p"
	.string "Their wings have never been stronger.\n"
	.string "Shall we see?$"

FortreeCity_Gym_Text_WinonaPreRematch4:
	.string "Winona: The sky over Fortree has\n"
	.string "changed. Wetter. Wilder.\p"
	.string "We've learned to fly through it\n"
	.string "instead of waiting for it to pass.\p"
	.string "Let us soar, Champion.$"

FortreeCity_Gym_Text_WinonaPreRematch5:
	.string "Winona: In every flight there is a\n"
	.string "moment when you stop fighting the\l"
	.string "wind and become part of it.\p"
	.string "That is the team you face now.\n"
	.string "Show me your wings!$"

FortreeCity_Gym_Text_WinonaRematchDefeatElite:
	.string "Grounded… but never broken.$"

FortreeCity_Gym_Text_WinonaPostRematchElite:
	.string "Winona: Wally read my strategy by the\n"
	.string "second turn. You read it before I'd\l"
	.string "even made it.\p"
	.string "Two different kinds of genius.\n"
	.string "I'm glad Hoenn has both.$"

@ Wally's Journey: growing Gym Leader rematches (see WallyJourney_ScaleLeaderParty).
FortreeCity_Gym_EventScript_WinonaRematchOffer::
	setvar VAR_0x8004, 6
	special WallyJourney_SetupLeaderRematch
	switch VAR_RESULT
	case 1, FortreeCity_Gym_EventScript_WinonaRematchStage1
	case 2, FortreeCity_Gym_EventScript_WinonaRematchStage2
	case 3, FortreeCity_Gym_EventScript_WinonaRematchStage3
	case 4, FortreeCity_Gym_EventScript_WinonaRematchStage4
	goto FortreeCity_Gym_EventScript_WinonaRematchStage1

FortreeCity_Gym_EventScript_WinonaRematchStage1::
	msgbox FortreeCity_Gym_Text_WinonaGrowOffer1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_Gym_EventScript_WinonaRematchDecline
	cleartrainerflag TRAINER_WINONA_2
	trainerbattle_no_intro TRAINER_WINONA_2, FortreeCity_Gym_Text_WinonaGrowDefeat1
	msgbox FortreeCity_Gym_Text_WinonaGrowAfter1, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_WinonaRematchStage2::
	msgbox FortreeCity_Gym_Text_WinonaGrowOffer2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_Gym_EventScript_WinonaRematchDecline
	cleartrainerflag TRAINER_WINONA_2
	trainerbattle_no_intro TRAINER_WINONA_2, FortreeCity_Gym_Text_WinonaGrowDefeat2
	msgbox FortreeCity_Gym_Text_WinonaGrowAfter2, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_WinonaRematchStage3::
	msgbox FortreeCity_Gym_Text_WinonaGrowOffer3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_Gym_EventScript_WinonaRematchDecline
	cleartrainerflag TRAINER_WINONA_3
	trainerbattle_double TRAINER_WINONA_3, FortreeCity_Gym_Text_WinonaGrowGo, FortreeCity_Gym_Text_WinonaGrowDefeat3, FortreeCity_Gym_Text_WinonaGrowNeedTwo
	setvar VAR_0x8004, 6
	special WallyJourney_RecordLeaderPostgameWin
	msgbox FortreeCity_Gym_Text_WinonaGrowAfter3, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_WinonaRematchStage4::
	msgbox FortreeCity_Gym_Text_WinonaGrowOffer4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_Gym_EventScript_WinonaRematchDecline
	cleartrainerflag TRAINER_WINONA_4
	trainerbattle_double TRAINER_WINONA_4, FortreeCity_Gym_Text_WinonaGrowGo, FortreeCity_Gym_Text_WinonaGrowDefeat4, FortreeCity_Gym_Text_WinonaGrowNeedTwo
	msgbox FortreeCity_Gym_Text_WinonaGrowAfter4, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_WinonaRematchDecline::
	msgbox FortreeCity_Gym_Text_WinonaPostBattle, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_Text_WinonaGrowGo:
	.string "Here we go!$"

FortreeCity_Gym_Text_WinonaGrowNeedTwo:
	.string "You need two Pokémon for this\n"
	.string "battle. Come back with more.$"

FortreeCity_Gym_Text_WinonaGrowOffer1:
	.string "Winona: You hold {STR_VAR_1} Badges now.\p"
	.string "A Leader's team is chosen to suit\n"
	.string "the challenger's Badges.\p"
	.string "Yours let us fly higher.\n"
	.string "Will you fly with us?$"

FortreeCity_Gym_Text_WinonaGrowDefeat1:
	.string "You soared past us again.$"

FortreeCity_Gym_Text_WinonaGrowAfter1:
	.string "Come back after your next Badge.\n"
	.string "We'll be training on the wind.$"

FortreeCity_Gym_Text_WinonaGrowOffer2:
	.string "Winona: Eight Badges. The League\n"
	.string "waits beyond Victory Road.\p"
	.string "We will hold back far less today.\n"
	.string "One more flight?$"

FortreeCity_Gym_Text_WinonaGrowDefeat2:
	.string "Your wings are ready.$"

FortreeCity_Gym_Text_WinonaGrowAfter2:
	.string "Fly to Ever Grande.\n"
	.string "The sky is yours.$"

FortreeCity_Gym_Text_WinonaGrowOffer3:
	.string "Winona: The Champion returns to\n"
	.string "Fortree.\p"
	.string "My full team will greet you.\n"
	.string "Will you battle?$"

FortreeCity_Gym_Text_WinonaGrowDefeat3:
	.string "Graceful and fierce.\n"
	.string "A true Champion.$"

FortreeCity_Gym_Text_WinonaGrowAfter3:
	.string "We'll keep training among\n"
	.string "the treetops.$"

FortreeCity_Gym_Text_WinonaGrowOffer4:
	.string "Winona: You defended your title.\p"
	.string "My team has reached its peak.\n"
	.string "Let us fly with everything we have.$"

FortreeCity_Gym_Text_WinonaGrowDefeat4:
	.string "No wind could lift us higher\n"
	.string "than you.$"

FortreeCity_Gym_Text_WinonaGrowAfter4:
	.string "Visit Fortree whenever you like.$"

