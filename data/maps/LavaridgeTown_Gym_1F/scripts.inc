LavaridgeTown_Gym_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LavaridgeTown_Gym_1F_OnTransition
	.byte 0

LavaridgeTown_Gym_1F_OnTransition:
	call LavaridgeTown_Gym_1F_EventScript_SetTrainerTempVars
	call LavaridgeTown_Gym_1F_EventScript_CheckBuryTrainers
	end

@ Unclear where/if these temp vars are getting checked
LavaridgeTown_Gym_1F_EventScript_SetTrainerTempVars::
	setvar VAR_TEMP_B, 0
	setvar VAR_TEMP_C, 0
	setvar VAR_TEMP_D, 0
	setvar VAR_TEMP_E, 0
	setvar VAR_TEMP_F, 0
	goto_if_defeated TRAINER_COLE, LavaridgeTown_Gym_1F_EventScript_SetGeraldTempVar
	setvar VAR_TEMP_B, 1
LavaridgeTown_Gym_1F_EventScript_SetGeraldTempVar::
	goto_if_defeated TRAINER_GERALD, LavaridgeTown_Gym_1F_EventScript_SetAxleTempVar
	setvar VAR_TEMP_C, 1
LavaridgeTown_Gym_1F_EventScript_SetAxleTempVar::
	goto_if_defeated TRAINER_AXLE, LavaridgeTown_Gym_1F_EventScript_SetDanielleTempVar
	setvar VAR_TEMP_D, 1
LavaridgeTown_Gym_1F_EventScript_SetDanielleTempVar::
	goto_if_defeated TRAINER_DANIELLE, LavaridgeTown_Gym_1F_EventScript_EndSetTrainerTempVars
	setvar VAR_TEMP_E, 1
LavaridgeTown_Gym_1F_EventScript_EndSetTrainerTempVars::
	return

LavaridgeTown_Gym_1F_EventScript_CheckBuryTrainers::
	goto_if_defeated TRAINER_COLE, LavaridgeTown_Gym_1F_EventScript_CheckBuryGerald
	setobjectmovementtype LOCALID_COLE, MOVEMENT_TYPE_BURIED
LavaridgeTown_Gym_1F_EventScript_CheckBuryGerald::
	goto_if_defeated TRAINER_GERALD, LavaridgeTown_Gym_1F_EventScript_CheckBuryAxle
	setobjectmovementtype LOCALID_GERALD, MOVEMENT_TYPE_BURIED
LavaridgeTown_Gym_1F_EventScript_CheckBuryAxle::
	goto_if_defeated TRAINER_AXLE, LavaridgeTown_Gym_1F_EventScript_CheckBuryDanielle
	setobjectmovementtype LOCALID_AXLE, MOVEMENT_TYPE_BURIED
LavaridgeTown_Gym_1F_EventScript_CheckBuryDanielle::
	goto_if_defeated TRAINER_DANIELLE, LavaridgeTown_Gym_1F_EventScript_EndCheckBuryTrainers
	setobjectmovementtype LOCALID_DANIELLE, MOVEMENT_TYPE_BURIED
LavaridgeTown_Gym_1F_EventScript_EndCheckBuryTrainers::
	return

LavaridgeTown_Gym_1F_EventScript_Flannery::
	trainerbattle_single TRAINER_FLANNERY_1, LavaridgeTown_Gym_1F_Text_FlanneryIntro, LavaridgeTown_Gym_1F_Text_FlanneryDefeat, LavaridgeTown_Gym_1F_EventScript_FlanneryDefeated, NO_MUSIC
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, LavaridgeTown_Gym_1F_EventScript_FlanneryRematch
	call WallyJourney_EventScript_GiveGymHMs
	goto_if_unset FLAG_RECEIVED_TM_OVERHEAT, LavaridgeTown_Gym_1F_EventScript_GiveOverheat2
	msgbox LavaridgeTown_Gym_1F_Text_FlanneryPostBattle, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_Gym_1F_EventScript_FlanneryDefeated::
	message LavaridgeTown_Gym_1F_Text_ReceivedHeatBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox LavaridgeTown_Gym_1F_Text_ExplainHeatBadgeTakeThis, MSGBOX_DEFAULT
	setflag FLAG_WHITEOUT_TO_LAVARIDGE
	setflag FLAG_DEFEATED_LAVARIDGE_GYM
	setflag FLAG_BADGE04_GET
	setvar VAR_WALLY_RIVAL_PENDING, 4
	addvar VAR_PETALBURG_GYM_STATE, 1
	call_if_eq VAR_PETALBURG_GYM_STATE, 6, Common_EventScript_ReadyPetalburgGymForBattle
	setvar VAR_0x8008, 4
	call Common_EventScript_SetGymTrainers
	setflag FLAG_HIDE_VERDANTURF_TOWN_WANDAS_HOUSE_WALLY
	setvar VAR_LAVARIDGE_TOWN_STATE, 1
	call LavaridgeTown_Gym_1F_EventScript_GiveOverheat
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox LavaridgeTown_Gym_1F_Text_RegisteredFlannery, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_ENABLE_FLANNERY_MATCH_CALL
	release
	end

LavaridgeTown_Gym_1F_EventScript_GiveOverheat2::
	giveitem ITEM_TM_OVERHEAT
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox LavaridgeTown_Gym_1F_Text_ExplainOverheat, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_OVERHEAT
	call WallyJourney_EventScript_GiveGymHMs
	release
	end

LavaridgeTown_Gym_1F_EventScript_GiveOverheat::
	giveitem ITEM_TM_OVERHEAT
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_BagIsFull
	msgbox LavaridgeTown_Gym_1F_Text_ExplainOverheat, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_OVERHEAT
	call WallyJourney_EventScript_GiveGymHMs
	return

LavaridgeTown_Gym_1F_EventScript_FlanneryRematch::
	setvar VAR_0x8004, TRAINER_FLANNERY_1
	special WallyJourney_GetRematchTier
	goto_if_eq VAR_RESULT, 3, LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier3
	goto_if_eq VAR_RESULT, 4, LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier4
	goto_if_ge VAR_RESULT, 5, LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier5
	trainerbattle_rematch_double TRAINER_FLANNERY_1, LavaridgeTown_Gym_1F_Text_FlanneryPreRematch, LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeat, LavaridgeTown_Gym_1F_Text_FlanneryRematchNeedTwoMons
	msgbox LavaridgeTown_Gym_1F_Text_FlanneryPostRematch, MSGBOX_AUTOCLOSE
	end

@ Wally's Journey: every rematch tier has its own opening line; the top two
@ tiers share a tougher defeat and post-battle line.
LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier3::
	trainerbattle_rematch_double TRAINER_FLANNERY_1, LavaridgeTown_Gym_1F_Text_FlanneryPreRematch3, LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeat, LavaridgeTown_Gym_1F_Text_FlanneryRematchNeedTwoMons
	msgbox LavaridgeTown_Gym_1F_Text_FlanneryPostRematch, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier4::
	trainerbattle_rematch_double TRAINER_FLANNERY_1, LavaridgeTown_Gym_1F_Text_FlanneryPreRematch4, LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeatElite, LavaridgeTown_Gym_1F_Text_FlanneryRematchNeedTwoMons
	msgbox LavaridgeTown_Gym_1F_Text_FlanneryPostRematchElite, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_FlanneryRematchTier5::
	trainerbattle_rematch_double TRAINER_FLANNERY_1, LavaridgeTown_Gym_1F_Text_FlanneryPreRematch5, LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeatElite, LavaridgeTown_Gym_1F_Text_FlanneryRematchNeedTwoMons
	msgbox LavaridgeTown_Gym_1F_Text_FlanneryPostRematchElite, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_Cole::
	trainerbattle_lavaridge LOCALID_COLE, TRAINER_COLE, LavaridgeTown_Gym_1F_Text_ColeIntro, LavaridgeTown_Gym_1F_Text_ColeDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	end

LavaridgeTown_Gym_EventScript_CheckTrainerScript::
	call LavaridgeTown_Gym_1F_EventScript_SetTrainerTempVars
	release
	special ShouldTryGetTrainerScript
	goto_if_eq VAR_RESULT, 1, EventScript_GotoTrainerScript
	end

LavaridgeTown_Gym_1F_EventScript_Axle::
	trainerbattle_lavaridge LOCALID_AXLE, TRAINER_AXLE, LavaridgeTown_Gym_1F_Text_AxleIntro, LavaridgeTown_Gym_1F_Text_AxleDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_1F_Text_AxlePostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_B1F_EventScript_Keegan::
	trainerbattle_lavaridge LOCALID_KEEGAN, TRAINER_KEEGAN, LavaridgeTown_Gym_B1F_Text_KeeganIntro, LavaridgeTown_Gym_B1F_Text_KeeganDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_B1F_Text_KeeganPostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_Danielle::
	trainerbattle_lavaridge LOCALID_DANIELLE, TRAINER_DANIELLE, LavaridgeTown_Gym_1F_Text_DanielleIntro, LavaridgeTown_Gym_1F_Text_DanielleDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_1F_Text_DaniellePostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_Gerald::
	trainerbattle_lavaridge LOCALID_GERALD, TRAINER_GERALD, LavaridgeTown_Gym_1F_Text_GeraldIntro, LavaridgeTown_Gym_1F_Text_GeraldDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_1F_Text_GeraldPostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_B1F_EventScript_Jace::
	trainerbattle_lavaridge LOCALID_JACE, TRAINER_JACE, LavaridgeTown_Gym_B1F_Text_JaceIntro, LavaridgeTown_Gym_B1F_Text_JaceDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_B1F_Text_JacePostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_B1F_EventScript_Jeff::
	trainerbattle_lavaridge LOCALID_JEFF, TRAINER_JEFF, LavaridgeTown_Gym_B1F_Text_JeffIntro, LavaridgeTown_Gym_B1F_Text_JeffDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_B1F_Text_JeffPostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_B1F_EventScript_Eli::
	trainerbattle_lavaridge LOCALID_ELI, TRAINER_ELI, LavaridgeTown_Gym_B1F_Text_EliIntro, LavaridgeTown_Gym_B1F_Text_EliDefeat, LavaridgeTown_Gym_EventScript_CheckTrainerScript
	msgbox LavaridgeTown_Gym_B1F_Text_EliPostBattle, MSGBOX_AUTOCLOSE
	end

LavaridgeTown_Gym_1F_EventScript_GymGuide::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_LAVARIDGE_GYM, LavaridgeTown_Gym_1F_EventScript_GymGuidePostVictory
	msgbox LavaridgeTown_Gym_1F_Text_GymGuideAdvice, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_Gym_1F_EventScript_GymGuidePostVictory::
	msgbox LavaridgeTown_Gym_1F_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_Gym_1F_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE04_GET, LavaridgeTown_Gym_1F_EventScript_GymStatueCertified
	goto LavaridgeTown_Gym_1F_EventScript_GymStatue
	end

LavaridgeTown_Gym_1F_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE04_GET, LavaridgeTown_Gym_1F_EventScript_GymStatueCertified
	goto LavaridgeTown_Gym_1F_EventScript_GymStatue
	end

LavaridgeTown_Gym_1F_EventScript_GymStatueCertified::
	msgbox LavaridgeTown_Gym_1F_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

LavaridgeTown_Gym_1F_EventScript_GymStatue::
	msgbox LavaridgeTown_Gym_1F_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

LavaridgeTown_Gym_1F_Text_GymGuideAdvice:
	.string "Hey, how's it going, CHAMPION-\n"
	.string "bound {PLAYER}?\p"
	.string "LAVARIDGE's GYM LEADER FLANNERY\n"
	.string "uses FIRE-type POKéMON.\p"
	.string "Her passion for POKéMON burns stronger\n"
	.string "and hotter than a volcano.\p"
	.string "Don't get too close to her--you'll burn!\n"
	.string "Hose her down with water and then\l"
	.string "go for it!$"

LavaridgeTown_Gym_1F_Text_GymGuidePostVictory:
	.string "Yow! That was a scorching-hot battle!\p"
	.string "FLANNERY only took over this GYM\n"
	.string "recently. The LEAGUE made her pass\l"
	.string "the same review as anyone else.$"

LavaridgeTown_Gym_1F_Text_ColeIntro:
	.string "Owowow! Yikes, it's hot!$"

LavaridgeTown_Gym_1F_Text_ColeDefeat:
	.string "Sweat in my eyes!\n"
	.string "I can't see a thing!$"

LavaridgeTown_Gym_1F_Text_ColePostBattle:
	.string "FLANNERY's grandfather built this GYM\n"
	.string "right on the hot springs.\p"
	.string "She sleeps here some nights, going\n"
	.string "through his old battle notes.$"

LavaridgeTown_Gym_1F_Text_AxleIntro:
	.string "I'm relieving stress. Don't stress me\n"
	.string "out!$"

LavaridgeTown_Gym_1F_Text_AxleDefeat:
	.string "I hope FLANNERY gives you\n"
	.string "a real roasting!$"

LavaridgeTown_Gym_1F_Text_AxlePostBattle:
	.string "FLANNERY practices her LEADER speech\n"
	.string "in the mirror every morning.\p"
	.string "She still messes it up.\n"
	.string "We all pretend not to notice.$"

LavaridgeTown_Gym_B1F_Text_KeeganIntro:
	.string "Tired? Want to rest in the hot sand?\n"
	.string "Too bad!$"

LavaridgeTown_Gym_B1F_Text_KeeganDefeat:
	.string "Played with fire…\n"
	.string "and got burned.$"

LavaridgeTown_Gym_B1F_Text_KeeganPostBattle:
	.string "Your skill is real. But FLANNERY is\n"
	.string "strong. Watch yourself.$"

LavaridgeTown_Gym_1F_Text_GeraldIntro:
	.string "Can your POKéMON stand 392 degrees?$"

LavaridgeTown_Gym_1F_Text_GeraldDefeat:
	.string "It didn't burn hot enough…$"

LavaridgeTown_Gym_1F_Text_GeraldPostBattle:
	.string "Magma is 392 degrees, give or take.\p"
	.string "TEAM MAGMA wants to play with that.\n"
	.string "Madness.$"

LavaridgeTown_Gym_1F_Text_DanielleIntro:
	.string "Um… Okay. I'll battle.$"

LavaridgeTown_Gym_1F_Text_DanielleDefeat:
	.string "Oh… You're too strong.$"

LavaridgeTown_Gym_1F_Text_DaniellePostBattle:
	.string "I'm going to be a pretty and strong\n"
	.string "TRAINER, just like FLANNERY.\p"
	.string "She was scared her first week too.\n"
	.string "She told me.$"

LavaridgeTown_Gym_B1F_Text_JaceIntro:
	.string "Come on! Before my fire cools!$"

LavaridgeTown_Gym_B1F_Text_JaceDefeat:
	.string "So hot in here, and my\n"
	.string "heart's frozen…$"

LavaridgeTown_Gym_B1F_Text_JacePostBattle:
	.string "That fighting spirit burning in you…\n"
	.string "You might beat our LEADER.$"

LavaridgeTown_Gym_B1F_Text_JeffIntro:
	.string "See the flames? They blaze for my\n"
	.string "win!$"

LavaridgeTown_Gym_B1F_Text_JeffDefeat:
	.string "Something went wrong.\n"
	.string "My coals went cold!$"

LavaridgeTown_Gym_B1F_Text_JeffPostBattle:
	.string "So what? I can walk on hot coals!\p"
	.string "…Don't try that.$"

LavaridgeTown_Gym_B1F_Text_EliIntro:
	.string "I love mountains. Volcanoes most of\n"
	.string "all.$"

LavaridgeTown_Gym_B1F_Text_EliDefeat:
	.string "I never had control of that\n"
	.string "battle. Not once.$"

LavaridgeTown_Gym_B1F_Text_EliPostBattle:
	.string "I stay because I'm a fan of\n"
	.string "FLANNERY's power.\p"
	.string "And because she stood up to TEAM\n"
	.string "MAGMA's survey crews. Alone.$"

LavaridgeTown_Gym_1F_Text_FlanneryIntro:
	.string "Welcome… No, wait.\p"
	.string "Puny TRAINER! How good to see you've\n"
	.string "made it here!\p"
	.string "I am FLANNERY, LEADER of this GYM!\n"
	.string "The… newest LEADER. In HOENN.\p"
	.string "My grandfather held this GYM for\n"
	.string "forty years. I've held it for four\l"
	.string "months.\p"
	.string "Uh… Don't underestimate me!\n"
	.string "I'll show you the fire of this\l"
	.string "mountain!$"

LavaridgeTown_Gym_1F_Text_FlanneryDefeat:
	.string "Oh…\n"
	.string "I was trying too hard again.\p"
	.string "I keep trying to be my grandfather.\n"
	.string "My POKéMON don't even know him.\p"
	.string "They know me. I should battle like\n"
	.string "me.\p"
	.string "Thanks for teaching me that.\n"
	.string "You deserve this.$"

LavaridgeTown_Gym_1F_Text_ReceivedHeatBadge:
	.string "{PLAYER} received the HEAT BADGE\n"
	.string "from FLANNERY.$"

LavaridgeTown_Gym_1F_Text_ExplainHeatBadgeTakeThis:
	.string "If you have a HEAT BADGE, all POKéMON\n"
	.string "up to Level 50, even those you get in\l"
	.string "trades from other people, will obey\l"
	.string "you completely.\p"
	.string "And, it lets POKéMON use the HM move\n"
	.string "STRENGTH outside of battle.\p"
	.string "This is a token of my appreciation.\n"
	.string "Don't be shy about taking it!$"

LavaridgeTown_Gym_1F_Text_ExplainOverheat:
	.string "That TM50 contains OVERHEAT.\p"
	.string "That move inflicts serious damage on\n"
	.string "the opponent.\p"
	.string "But it also sharply cuts the SP. ATK\n"
	.string "of the POKéMON using it. It might not\l"
	.string "be suitable for longer battles.$"

LavaridgeTown_Gym_1F_Text_RegisteredFlannery:
	.string "Registered GYM LEADER FLANNERY\n"
	.string "in the POKéNAV.$"

LavaridgeTown_Gym_1F_Text_FlanneryPostBattle:
	.string "Your power reminds me of someone…\p"
	.string "Oh! I know! You battle like NORMAN,\n"
	.string "the GYM LEADER of PETALBURG.\p"
	.string "You know WALLY, right? He lost to me\n"
	.string "twice. He came back a third time\l"
	.string "with a plan, and burned me out.\p"
	.string "I've never seen anyone learn from\n"
	.string "a loss that fast.$"

LavaridgeTown_Gym_1F_Text_GymStatue:
	.string "LAVARIDGE TOWN POKéMON GYM$"

LavaridgeTown_Gym_1F_Text_GymStatueCertified:
	.string "LAVARIDGE TOWN POKéMON GYM\p"
	.string "FLANNERY'S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

LavaridgeTown_Gym_1F_Text_FlanneryPreRematch:
	.string "FLANNERY: You came back!\p"
	.string "Grandpa said real LEADERS want the\n"
	.string "rematch more than the win.\p"
	.string "I finally get what he meant.\n"
	.string "Let's go!$"

LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeat:
	.string "Whew!\n"
	.string "That was hotter than the springs!$"

LavaridgeTown_Gym_1F_Text_FlanneryPostRematch:
	.string "FLANNERY: I lost, and I feel great!\p"
	.string "Grandpa always said the mountain\n"
	.string "teaches patience. I'm learning.\l"
	.string "Slowly!$"

LavaridgeTown_Gym_1F_Text_FlanneryRematchNeedTwoMons:
	.string "FLANNERY: Losing won't deflate me!\p"
	.string "Oh, wait. You only have one POKéMON\n"
	.string "that can battle?\p"
	.string "Rematches here are two at a time.\n"
	.string "Bring at least two!$"

LavaridgeTown_Gym_1F_Text_FlanneryPreRematch3:
	.string "FLANNERY: I stopped trying to battle\n"
	.string "like my grandfather.\p"
	.string "Now I battle like me.\n"
	.string "And I'm way hotter. Let's go!$"

LavaridgeTown_Gym_1F_Text_FlanneryPreRematch4:
	.string "FLANNERY: That volcano nearly blew\n"
	.string "because people thought they could\l"
	.string "control it.\p"
	.string "My fire's different. I know exactly\n"
	.string "how hot it burns. Here it comes!$"

LavaridgeTown_Gym_1F_Text_FlanneryPreRematch5:
	.string "FLANNERY: This is everything I've\n"
	.string "got. No nerves. No holding back.\p"
	.string "If you win, I'll treat the whole\n"
	.string "GYM to the hot springs.\p"
	.string "If I win… actually, same deal!$"

LavaridgeTown_Gym_1F_Text_FlanneryRematchDefeatElite:
	.string "Burned out… and I loved every second!$"

LavaridgeTown_Gym_1F_Text_FlanneryPostRematchElite:
	.string "FLANNERY: Grandpa called after our\n"
	.string "last match.\p"
	.string "He said I finally look like a LEADER.\n"
	.string "I told him I learned it from losing\l"
	.string "to you.\p"
	.string "He laughed for ages.$"
