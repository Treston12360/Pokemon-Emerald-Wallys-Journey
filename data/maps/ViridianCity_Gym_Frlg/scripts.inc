ViridianCity_Gym_Frlg_MapScripts::
	.byte 0

ViridianCity_Gym_EventScript_Giovanni::
	trainerbattle_single TRAINER_LEADER_BLUE, ViridianCity_Gym_Text_GiovanniIntro, ViridianCity_Gym_Text_GiovanniDefeat, ViridianCity_Gym_EventScript_DefeatedGiovanni, NO_MUSIC
	goto_if_unset FLAG_GOT_TM26_FROM_GIOVANNI, ViridianCity_Gym_EventScript_GiveTM26
	setspeaker SP_NAME_BLUE
	msgbox ViridianCity_Gym_Text_GiovanniPostBattle
	release
	end

ViridianCity_Gym_EventScript_DefeatedGiovanni::
	setflag FLAG_HIDE_MISC_KANTO_ROCKETS
	setflag FLAG_DEFEATED_LEADER_GIOVANNI
	setflag FLAG_BADGE08_GET
	setvar VAR_MAP_SCENE_ROUTE22, 3
	set_gym_trainers_frlg 8
	goto ViridianCity_Gym_EventScript_GiveTM26
	end

ViridianCity_Gym_EventScript_GiveTM26::
	setspeaker SP_NAME_BLUE
	msgbox ViridianCity_Gym_Text_ExplainEarthBadgeTakeThis
	checkitemspace ITEM_TM26
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_Gym_EventScript_NoRoomForTM26
	giveitem_msg ViridianCity_Gym_Text_ReceivedTM26FromGiovanni, ITEM_TM26
	setflag FLAG_GOT_TM26_FROM_GIOVANNI
	setspeaker SP_NAME_BLUE
	msgbox ViridianCity_Gym_Text_ExplainTM26
	release
	end

ViridianCity_Gym_EventScript_NoRoomForTM26::
	msgbox ViridianCity_Gym_Text_YouDoNotHaveSpace
	release
	end

ViridianCity_Gym_EventScript_Jason::
	trainerbattle_single TRAINER_TAMER_JASON, ViridianCity_Gym_Text_JasonIntro, ViridianCity_Gym_Text_JasonDefeat
	msgbox ViridianCity_Gym_Text_JasonPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Cole::
	trainerbattle_single TRAINER_TAMER_COLE, ViridianCity_Gym_Text_ColeIntro, ViridianCity_Gym_Text_ColeDefeat
	msgbox ViridianCity_Gym_Text_ColePostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Atsushi::
	trainerbattle_single TRAINER_BLACK_BELT_ATSUSHI, ViridianCity_Gym_Text_AtsushiIntro, ViridianCity_Gym_Text_AtsushiDefeat
	msgbox ViridianCity_Gym_Text_AtsushiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Kiyo::
	trainerbattle_single TRAINER_BLACK_BELT_KIYO, ViridianCity_Gym_Text_KiyoIntro, ViridianCity_Gym_Text_KiyoDefeat
	msgbox ViridianCity_Gym_Text_KiyoPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Takashi::
	trainerbattle_single TRAINER_BLACK_BELT_TAKASHI, ViridianCity_Gym_Text_TakashiIntro, ViridianCity_Gym_Text_TakashiDefeat
	msgbox ViridianCity_Gym_Text_TakashiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Samuel::
	trainerbattle_single TRAINER_COOLTRAINER_SAMUEL, ViridianCity_Gym_Text_SamuelIntro, ViridianCity_Gym_Text_SamuelDefeat
	msgbox ViridianCity_Gym_Text_SamuelPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Yuji::
	trainerbattle_single TRAINER_COOLTRAINER_YUJI, ViridianCity_Gym_Text_YujiIntro, ViridianCity_Gym_Text_YujiDefeat
	msgbox ViridianCity_Gym_Text_YujiPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_Warren::
	trainerbattle_single TRAINER_COOLTRAINER_WARREN, ViridianCity_Gym_Text_WarrenIntro, ViridianCity_Gym_Text_WarrenDefeat
	msgbox ViridianCity_Gym_Text_WarrenPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianCity_Gym_EventScript_GymGuy::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_LEADER_GIOVANNI, ViridianCity_Gym_EventScript_GymGuyPostVictory
	msgbox ViridianCity_Gym_Text_GymGuyAdvice
	release
	end

ViridianCity_Gym_EventScript_GymGuyPostVictory::
	famechecker FAMECHECKER_GIOVANNI, 4
	msgbox ViridianCity_Gym_Text_GymGuyPostVictory
	release
	end

ViridianCity_Gym_EventScript_GymStatue::
	lockall
	goto_if_set FLAG_BADGE08_GET, ViridianCity_Gym_EventScript_GymStatuePostVictory
	msgbox ViridianCity_Gym_Text_GymStatue
	releaseall
	end

ViridianCity_Gym_EventScript_GymStatuePostVictory::
	msgbox ViridianCity_Gym_Text_GymStatuePlayerWon
	releaseall
	end

ViridianCity_Gym_Text_GiovanniIntro:: @ ARIA S-LEAGUE draft v2 (docs/KANTO_SCRIPT.md L.1)
	.string "So you made it. Seven badges. I\n"
	.string "figured you'd quit somewhere around\l"
	.string "Celadon.\p"
	.string "This used to be Giovanni's Gym. Team\n"
	.string "Rocket's boss, running a League Gym,\l"
	.string "and nobody noticed.\p"
	.string "When the League asked me to take it\n"
	.string "over, I said no three times.\p"
	.string "Gramps finally talked me into it. He\n"
	.string "said a Gym should belong to someone\l"
	.string "who lost everything here and still\l"
	.string "came back.\p"
	.string "......\p"
	.string "I lost everything in that Champion's\n"
	.string "room once. I've spent a long time\l"
	.string "getting it back.\p"
	.string "I've been watching you since Mt.\n"
	.string "Moon. You don't fight like a\l"
	.string "tourist.\p"
	.string "So no holding back. This is my Gym,\n"
	.string "and I'm not the guy who lost to Red\l"
	.string "any more.$"

ViridianCity_Gym_Text_GiovanniDefeat:: @ ARIA S-LEAGUE draft v2 (docs/KANTO_SCRIPT.md L.1)
	.string "......Yeah. Okay. I get it now.\n"
	.string "That's what Gramps saw in you.\p"
	.string "{PLAYER} received the Earth Badge\n"
	.string "from Blue!{PAUSE_MUSIC}{PLAY_BGM}{MUS_OBTAIN_BADGE}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}$"

ViridianCity_Gym_Text_GiovanniPostBattle:: @ ARIA S-LEAGUE draft v2 (docs/KANTO_SCRIPT.md L.1)
	.string "Don't make that face. Losing to you\n"
	.string "doesn't feel like losing to Red did.\p"
	.string "Go win it. Then tell Gramps and\n"
	.string "Varnell yourself. They'll want to\l"
	.string "hear it from you.$"

ViridianCity_Gym_Text_ExplainEarthBadgeTakeThis:: @ ARIA S-LEAGUE draft v2 (docs/KANTO_SCRIPT.md L.1)
	.string "Here. The Earth Badge. That's all\n"
	.string "eight.\p"
	.string "With all eight, the League will let\n"
	.string "you onto Victory Road.\p"
	.string "Take this too.$"

ViridianCity_Gym_Text_ReceivedTM26FromGiovanni::
	.string "{PLAYER} received TM26\n"
	.string "from Blue.$"

ViridianCity_Gym_Text_ExplainTM26::
	.string "TM26 contains Earthquake.$"

ViridianCity_Gym_Text_YouDoNotHaveSpace::
	.string "You do not have space for this!$"

ViridianCity_Gym_Text_YujiIntro::
	.string "Heh!\n"
	.string "You must be running out of steam\l"
	.string "by now!$"

ViridianCity_Gym_Text_YujiDefeat::
	.string "I ran out of gas!$"

ViridianCity_Gym_Text_YujiPostBattle::
	.string "You'll need power to keep up with\n"
	.string "our Gym Leader.$"

ViridianCity_Gym_Text_AtsushiIntro::
	.string "Rrrroar!\n"
	.string "I'm working myself into a rage!$"

ViridianCity_Gym_Text_AtsushiDefeat::
	.string "Wargh!$"

ViridianCity_Gym_Text_AtsushiPostBattle::
	.string "I'm still not worthy!$"

ViridianCity_Gym_Text_JasonIntro::
	.string "Pokémon and I, we make wonderful\n"
	.string "music together!$"

ViridianCity_Gym_Text_JasonDefeat::
	.string "You are in perfect harmony!$"

ViridianCity_Gym_Text_JasonPostBattle::
	.string "Do you know the identity of our\n"
	.string "Gym Leader?$"

ViridianCity_Gym_Text_KiyoIntro::
	.string "Karate is the ultimate form of\n"
	.string "martial arts!$"

ViridianCity_Gym_Text_KiyoDefeat::
	.string "Aiyah!$"

ViridianCity_Gym_Text_KiyoPostBattle::
	.string "If my Pokémon were as good at\n"
	.string "karate as I…$"

ViridianCity_Gym_Text_WarrenIntro::
	.string "The truly talented win with style.$"

ViridianCity_Gym_Text_WarrenDefeat::
	.string "I lost my grip!$"

ViridianCity_Gym_Text_WarrenPostBattle::
	.string "The Leader will scold me for\n"
	.string "losing this way…$"

ViridianCity_Gym_Text_TakashiIntro::
	.string "I'm the Karate King!\n"
	.string "Your fate rests with me!$"

ViridianCity_Gym_Text_TakashiDefeat::
	.string "Ayah!$"

ViridianCity_Gym_Text_TakashiPostBattle::
	.string "The Pokémon League?\n"
	.string "You? Don't get cocky!$"

ViridianCity_Gym_Text_ColeIntro::
	.string "Your Pokémon will cower at the\n"
	.string "crack of my whip!$"

ViridianCity_Gym_Text_ColeDefeat::
	.string "Yowch!\n"
	.string "Whiplash!$"

ViridianCity_Gym_Text_ColePostBattle::
	.string "Wait!\n"
	.string "I was just careless!$"

ViridianCity_Gym_Text_SamuelIntro::
	.string "Viridian Gym was closed for a\n"
	.string "long time.\p"
	.string "But now, our Leader is back!$"

ViridianCity_Gym_Text_SamuelDefeat::
	.string "I was beaten?$"

ViridianCity_Gym_Text_SamuelPostBattle::
	.string "You can go on to the Pokémon\n"
	.string "League only by defeating our Gym\l"
	.string "Leader!$"

ViridianCity_Gym_Text_GymGuyAdvice::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "Even I don't know the Viridian\n"
	.string "Leader's identity.\p"
	.string "But one thing's certain.\n"
	.string "This will be the toughest of all\l"
	.string "the Gym Leaders.\p"
	.string "Also, I heard that the Trainers\n"
	.string "here like Ground-type Pokémon.$"

ViridianCity_Gym_Text_GymGuyPostVictory::
	.string "Blow me away! Giovanni was the\n"
	.string "Gym Leader of Viridian?$"

ViridianCity_Gym_Text_GymStatue::
	.string "Viridian Pokémon Gym\n"
	.string "Leader: ?\p"
	.string "Winning Trainers:\n"
	.string "{RIVAL}$"

ViridianCity_Gym_Text_GymStatuePlayerWon::
	.string "Viridian Pokémon Gym\n"
	.string "Leader: Giovanni\p"
	.string "Winning Trainers:\n"
	.string "{RIVAL}, {PLAYER}$"

