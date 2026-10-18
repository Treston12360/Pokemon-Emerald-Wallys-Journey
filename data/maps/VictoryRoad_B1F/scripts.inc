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
	.string "Getting closer to the POKéMON\n"
	.string "LEAGUE… I'm getting stage fright…$"

VictoryRoad_B1F_Text_SamuelDefeat:
	.string "I couldn't do a thing…$"

VictoryRoad_B1F_Text_SamuelPostBattle:
	.string "The LEAGUE feels far away again…\p"
	.string "My parents mortgaged the house for\n"
	.string "my training. How do I tell them?$"

VictoryRoad_B1F_Text_ShannonIntro:
	.string "To win through the LEAGUE, you need\n"
	.string "your POKéMON's trust.$"

VictoryRoad_B1F_Text_ShannonDefeat:
	.string "Your bond is built on\n"
	.string "solid trust.$"

VictoryRoad_B1F_Text_ShannonPostBattle:
	.string "PHOEBE of the ELITE FOUR grew up\n"
	.string "on MT. PYRE.\p"
	.string "She says GHOST POKéMON only trust\n"
	.string "people who've lost something.$"

VictoryRoad_B1F_Text_MichelleIntro:
	.string "This isn't the goal. It's only a\n"
	.string "stop on the way to the LEAGUE.$"

VictoryRoad_B1F_Text_MichelleDefeat:
	.string "That's the way!$"

VictoryRoad_B1F_Text_MichellePostBattle:
	.string "You'll do fine!\p"
	.string "One warning: SIDNEY talks tough, but\n"
	.string "he's the fairest of the four.\p"
	.string "It's the quiet ones after him you\n"
	.string "should worry about.$"

VictoryRoad_B1F_Text_MitchellIntro:
	.string "My POKéMON are cosmically\n"
	.string "awe-inspiring!$"

VictoryRoad_B1F_Text_MitchellDefeat:
	.string "I've never met anyone like you.$"

VictoryRoad_B1F_Text_MitchellPostBattle:
	.string "Even outside battle, I sense huge\n"
	.string "power from you and your POKéMON.$"

VictoryRoad_B1F_Text_HalleIntro:
	.string "Okay, no need to get your back up!\n"
	.string "Relax, let's take it easy!$"

VictoryRoad_B1F_Text_HalleDefeat:
	.string "Whoa!\n"
	.string "Wonderful!$"

VictoryRoad_B1F_Text_HallePostBattle:
	.string "Sure, this is VICTORY ROAD.\p"
	.string "But it's not that different from the\n"
	.string "path you've taken so far.\p"
	.string "Try to enjoy the rest of the way!$"

