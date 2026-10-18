VictoryRoad_B1F_MapScripts::
	.byte 0

VictoryRoad_B1F_EventScript_Samuel::
	trainerbattle_single TRAINER_SAMUEL, VictoryRoad_B1F_Text_SamuelIntro, VictoryRoad_B1F_Text_SamuelDefeat
	msgbox VictoryRoad_B1F_Text_SamuelPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B1F_EventScript_Shannon::
	trainerbattle_single TRAINER_SHANNON, VictoryRoad_B1F_Text_ShannonIntro, VictoryRoad_B1F_Text_ShannonDefeat
	msgbox VictoryRoad_B1F_Text_ShannonPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B1F_EventScript_Michelle::
	trainerbattle_single TRAINER_MICHELLE, VictoryRoad_B1F_Text_MichelleIntro, VictoryRoad_B1F_Text_MichelleDefeat
	msgbox VictoryRoad_B1F_Text_MichellePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B1F_EventScript_Mitchell::
	trainerbattle_single TRAINER_MITCHELL, VictoryRoad_B1F_Text_MitchellIntro, VictoryRoad_B1F_Text_MitchellDefeat
	msgbox VictoryRoad_B1F_Text_MitchellPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B1F_EventScript_Halle::
	trainerbattle_single TRAINER_HALLE, VictoryRoad_B1F_Text_HalleIntro, VictoryRoad_B1F_Text_HalleDefeat
	msgbox VictoryRoad_B1F_Text_HallePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B1F_Text_SamuelIntro:
	.string "The LEAGUE is right there…\n"
	.string "I can't stop shaking…$"

VictoryRoad_B1F_Text_SamuelDefeat:
	.string "I froze. I couldn't\n"
	.string "do anything…$"

VictoryRoad_B1F_Text_SamuelPostBattle:
	.string "The LEAGUE feels far away again…\p"
	.string "My parents mortgaged the house for\n"
	.string "my training. How do I tell them?$"

VictoryRoad_B1F_Text_ShannonIntro:
	.string "The LEAGUE tests trust more than\n"
	.string "strength. Let me test yours!$"

VictoryRoad_B1F_Text_ShannonDefeat:
	.string "They trust you completely.\n"
	.string "I could feel it.$"

VictoryRoad_B1F_Text_ShannonPostBattle:
	.string "PHOEBE of the ELITE FOUR grew up\n"
	.string "on MT. PYRE.\p"
	.string "She says GHOST POKéMON only trust\n"
	.string "people who've lost something.$"

VictoryRoad_B1F_Text_MichelleIntro:
	.string "This cave isn't the finish line.\n"
	.string "It's just the hallway!$"

VictoryRoad_B1F_Text_MichelleDefeat:
	.string "That's the way!\n"
	.string "Now go win!$"

VictoryRoad_B1F_Text_MichellePostBattle:
	.string "You'll do fine!\p"
	.string "One warning: SIDNEY talks tough, but\n"
	.string "he's the fairest of the four.\p"
	.string "It's the quiet ones after him you\n"
	.string "should worry about.$"

VictoryRoad_B1F_Text_MitchellIntro:
	.string "I've trained my POKéMON under the\n"
	.string "stars of MOSSDEEP! Behold!$"

VictoryRoad_B1F_Text_MitchellDefeat:
	.string "The stars didn't warn me\n"
	.string "about you.$"

VictoryRoad_B1F_Text_MitchellPostBattle:
	.string "Even standing still, you and your\n"
	.string "POKéMON hum with power.$"

VictoryRoad_B1F_Text_HalleIntro:
	.string "Whoa, relax! No need to be tense.\n"
	.string "Let's have fun with this!$"

VictoryRoad_B1F_Text_HalleDefeat:
	.string "Whoa! Wonderful!$"

VictoryRoad_B1F_Text_HallePostBattle:
	.string "Sure, it's VICTORY ROAD. But it's\n"
	.string "the same path you've walked\l"
	.string "this whole time.\p"
	.string "Enjoy the rest of it!$"

