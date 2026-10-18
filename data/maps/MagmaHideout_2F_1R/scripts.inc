MagmaHideout_2F_1R_MapScripts::
	.byte 0

MagmaHideout_2F_1R_EventScript_Grunt14::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_14, MagmaHideout_2F_1R_Text_Grunt14Intro, MagmaHideout_2F_1R_Text_Grunt14Defeat
	msgbox MagmaHideout_2F_1R_Text_Grunt14PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_1R_EventScript_Grunt3::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_3, MagmaHideout_2F_1R_Text_Grunt3Intro, MagmaHideout_2F_1R_Text_Grunt3Defeat
	msgbox MagmaHideout_2F_1R_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_1R_EventScript_Grunt4::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_4, MagmaHideout_2F_1R_Text_Grunt4Intro, MagmaHideout_2F_1R_Text_Grunt4Defeat
	msgbox MagmaHideout_2F_1R_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_1R_EventScript_Grunt5::
	trainerbattle_single TRAINER_GRUNT_MAGMA_HIDEOUT_5, MagmaHideout_2F_1R_Text_Grunt5Intro, MagmaHideout_2F_1R_Text_Grunt5Defeat
	msgbox MagmaHideout_2F_1R_Text_Grunt5PostBattle, MSGBOX_AUTOCLOSE
	end

MagmaHideout_2F_1R_Text_Grunt14Intro:
	.string "What, what, what?\p"
	.string "Only TEAM MAGMA members belong\n"
	.string "in here!\p"
	.string "You're not in uniform…\n"
	.string "You rouse my suspicion! Battle!$"

MagmaHideout_2F_1R_Text_Grunt14Defeat:
	.string "Aiyiyi…\n"
	.string "My pride as a MAGMA member…$"

MagmaHideout_2F_1R_Text_Grunt14PostBattle:
	.string "I used to swing a hammer on the\n"
	.string "LAND INITIATIVE's building days.\p"
	.string "We put up twelve houses in\n"
	.string "FALLARBOR. I miss that.$"

MagmaHideout_2F_1R_Text_Grunt3Intro:
	.string "Hold it! You think I'll keep my\n"
	.string "trap shut and let you by?$"

MagmaHideout_2F_1R_Text_Grunt3Defeat:
	.string "Ooh wow!\n"
	.string "I concede!$"

MagmaHideout_2F_1R_Text_Grunt3PostBattle:
	.string "The scientists said the eruption\n"
	.string "model was wrong. MAXIE called them\l"
	.string "cowards.\p"
	.string "I didn't say anything. Maybe I'm\n"
	.string "the coward.$"

MagmaHideout_2F_1R_Text_Grunt4Intro:
	.string "Ahah!\n"
	.string "An intruder!$"

MagmaHideout_2F_1R_Text_Grunt4Defeat:
	.string "Graaah!$"

MagmaHideout_2F_1R_Text_Grunt4PostBattle:
	.string "I already lost.\n"
	.string "Do I still have to run in circles?$"

MagmaHideout_2F_1R_Text_Grunt5Intro:
	.string "Oh, oh!\n"
	.string "An intruder!$"

MagmaHideout_2F_1R_Text_Grunt5Defeat:
	.string "Mutter…$"

MagmaHideout_2F_1R_Text_Grunt5PostBattle:
	.string "Actually, I'm no expert at battling…\p"
	.string "I'm a surveyor. I joined to help\n"
	.string "plan new towns. Not to wake up a\l"
	.string "volcano god.$"

