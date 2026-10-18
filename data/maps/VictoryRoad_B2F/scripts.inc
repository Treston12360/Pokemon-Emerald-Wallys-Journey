VictoryRoad_B2F_MapScripts::
	.byte 0

VictoryRoad_B2F_EventScript_Vito::
	trainerbattle_single TRAINER_VITO, VictoryRoad_B2F_Text_VitoIntro, VictoryRoad_B2F_Text_VitoDefeat
	msgbox VictoryRoad_B2F_Text_VitoPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Owen::
	trainerbattle_single TRAINER_OWEN, VictoryRoad_B2F_Text_OwenIntro, VictoryRoad_B2F_Text_OwenDefeat
	msgbox VictoryRoad_B2F_Text_OwenPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Caroline::
	trainerbattle_single TRAINER_CAROLINE, VictoryRoad_B2F_Text_CarolineIntro, VictoryRoad_B2F_Text_CarolineDefeat
	msgbox VictoryRoad_B2F_Text_CarolinePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Julie::
	trainerbattle_single TRAINER_JULIE, VictoryRoad_B2F_Text_JulieIntro, VictoryRoad_B2F_Text_JulieDefeat
	msgbox VictoryRoad_B2F_Text_JuliePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Felix::
	trainerbattle_single TRAINER_FELIX, VictoryRoad_B2F_Text_FelixIntro, VictoryRoad_B2F_Text_FelixDefeat
	msgbox VictoryRoad_B2F_Text_FelixPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Dianne::
	trainerbattle_single TRAINER_DIANNE, VictoryRoad_B2F_Text_DianneIntro, VictoryRoad_B2F_Text_DianneDefeat
	msgbox VictoryRoad_B2F_Text_DiannePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_Text_VitoIntro:
	.string "I trained with my whole family,\n"
	.string "every one of us! I won't lose!$"

VictoryRoad_B2F_Text_VitoDefeat:
	.string "Better than my family?!\n"
	.string "Is that possible?!$"

VictoryRoad_B2F_Text_VitoPostBattle:
	.string "I was the best in my family.\n"
	.string "I'd never lost…\p"
	.string "Maybe I'll go home. They'll be glad\n"
	.string "to see me either way. I hope.$"

VictoryRoad_B2F_Text_OwenIntro:
	.string "I heard there's a tough little kid\n"
	.string "around. Is that you?$"

VictoryRoad_B2F_Text_OwenDefeat:
	.string "The little shrimp is tough!$"

VictoryRoad_B2F_Text_OwenPostBattle:
	.string "The rumor says that tough kid is\n"
	.string "from PETALBURG CITY.\p"
	.string "Must be something in the water\n"
	.string "over there.$"

VictoryRoad_B2F_Text_CarolineIntro:
	.string "You must be getting a little tired.$"

VictoryRoad_B2F_Text_CarolineDefeat:
	.string "No signs of tiring at all!$"

VictoryRoad_B2F_Text_CarolinePostBattle:
	.string "VICTORY ROAD and the LEAGUE are long\n"
	.string "and grueling. Beware of fatigue!\p"
	.string "GLACIA came all the way from abroad\n"
	.string "for this. She hasn't gone home since.$"

VictoryRoad_B2F_Text_JulieIntro:
	.string "Don't get complacent just because\n"
	.string "you have lots of BADGES.\p"
	.string "There's always someone better!$"

VictoryRoad_B2F_Text_JulieDefeat:
	.string "You're better than me!$"

VictoryRoad_B2F_Text_JuliePostBattle:
	.string "Look at your BADGES and remember\n"
	.string "the TRAINERS you've faced.$"

VictoryRoad_B2F_Text_FelixIntro:
	.string "I've come this far, but the tension\n"
	.string "is giving me awful stomach pain…$"

VictoryRoad_B2F_Text_FelixDefeat:
	.string "Ooh…\n"
	.string "It hurts…$"

VictoryRoad_B2F_Text_FelixPostBattle:
	.string "I get so tense this close to the\n"
	.string "LEAGUE.\p"
	.string "DRAKE was a sailor before he was\n"
	.string "ELITE FOUR. Rode out storms nobody\l"
	.string "else would.\p"
	.string "I can't even ride out a cave.$"

VictoryRoad_B2F_Text_DianneIntro:
	.string "The elite of the elite gather in\n"
	.string "this cave. How are you finding it?$"

VictoryRoad_B2F_Text_DianneDefeat:
	.string "Not rattled in the least!$"

VictoryRoad_B2F_Text_DiannePostBattle:
	.string "You've got guts! I like that!\n"
	.string "Keep it going!$"

