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
	.string "If you came through Route 111, you\n"
	.string "might've met my family. The Winstrates.\p"
	.string "I'm Vito. Victor and Victoria are my\n"
	.string "parents, Vivi's my little sister, and\l"
	.string "Grandma Vicky taught me everything I\l"
	.string "know.\p"
	.string "Grandma always said I'd be Champion\n"
	.string "someday.\p"
	.string "Every Pokémon on my team carries\n"
	.string "something from home.\p"
	.string "Let's see how you handle the whole\n"
	.string "Winstrate family at once!$"

VictoryRoad_B2F_Text_VitoDefeat:
	.string "No way… The pride of the Winstrates,\n"
	.string "beaten.$"

VictoryRoad_B2F_Text_VitoPostBattle:
	.string "I'm not going home until I've beaten\n"
	.string "the League.\p"
	.string "If you pass through Route 111, tell\n"
	.string "Grandma I'm still training.\p"
	.string "…And don't tell her I lost.$"

VictoryRoad_B2F_Text_OwenIntro:
	.string "A kid from Petalburg coming through?\p"
	.string "Norman's kid? Huh. Is that you?$"

VictoryRoad_B2F_Text_OwenDefeat:
	.string "The little shrimp is\n"
	.string "the real deal!$"

VictoryRoad_B2F_Text_OwenPostBattle:
	.string "The rumor says that tough kid is\n"
	.string "from Petalburg City.\p"
	.string "Must be something in the water\n"
	.string "over there.$"

VictoryRoad_B2F_Text_CarolineIntro:
	.string "Three floors. You must be tired.\p"
	.string "Steven made it through in one night,\n"
	.string "they say. I've been here a week!$"

VictoryRoad_B2F_Text_CarolineDefeat:
	.string "Not tired at all?!\n"
	.string "What are you?$"

VictoryRoad_B2F_Text_CarolinePostBattle:
	.string "Victory Road and the League are long\n"
	.string "and grueling. Beware of fatigue!\p"
	.string "Glacia came all the way from abroad\n"
	.string "for this. She hasn't gone home since.$"

VictoryRoad_B2F_Text_JulieIntro:
	.string "Lots of Badges? Don't get cocky.\p"
	.string "Wally had all eight too. Someone's\n"
	.string "always better!$"

VictoryRoad_B2F_Text_JulieDefeat:
	.string "…Turns out that someone\n"
	.string "is you.$"

VictoryRoad_B2F_Text_JuliePostBattle:
	.string "Look at your Badges. Remember every\n"
	.string "Trainer behind each one.$"

VictoryRoad_B2F_Text_FelixIntro:
	.string "So close to the League…\n"
	.string "My stomach is in knots…$"

VictoryRoad_B2F_Text_FelixDefeat:
	.string "Ooh… my stomach…$"

VictoryRoad_B2F_Text_FelixPostBattle:
	.string "I get so tense this close to the\n"
	.string "League.\p"
	.string "Drake was a sailor before he was\n"
	.string "Elite Four. Rode out storms nobody\l"
	.string "else would.\p"
	.string "I can't even ride out a cave.$"

VictoryRoad_B2F_Text_DianneIntro:
	.string "Only the best of the best reach this\n"
	.string "cave.\p"
	.string "I watched Wally's title match from\n"
	.string "here on a radio. Feeling it?$"

VictoryRoad_B2F_Text_DianneDefeat:
	.string "Not even nervous!\n"
	.string "Impressive!$"

VictoryRoad_B2F_Text_DiannePostBattle:
	.string "You've got guts. The Elite Four\n"
	.string "will like that.$"

