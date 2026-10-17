PacifidlogTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PacifidlogTown_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, PacifidlogTown_OnResume
	.byte 0

PacifidlogTown_OnTransition:
	setflag FLAG_VISITED_PACIFIDLOG_TOWN
	end

PacifidlogTown_OnResume:
	setstepcallback STEP_CB_PACIFIDLOG_BRIDGE
	end

PacifidlogTown_EventScript_NinjaBoy::
	msgbox PacifidlogTown_Text_NeatHousesOnWater, MSGBOX_NPC
	end

PacifidlogTown_EventScript_Girl::
	goto_if_set FLAG_DEFEATED_SOOTOPOLIS_GYM, PacifidlogTown_Text_FastRunningCurrentAftermath_Script
	msgbox PacifidlogTown_Text_FastRunningCurrent, MSGBOX_NPC
	end

PacifidlogTown_Text_FastRunningCurrentAftermath_Script:
	msgbox PacifidlogTown_Text_FastRunningCurrentAftermath, MSGBOX_NPC
	end

PacifidlogTown_EventScript_Fisherman::
	msgbox PacifidlogTown_Text_SkyPillarTooScary, MSGBOX_NPC
	end

PacifidlogTown_EventScript_TownSign::
	msgbox PacifidlogTown_Text_TownSign, MSGBOX_SIGN
	end

PacifidlogTown_Text_FastRunningCurrent:
	.string "The sea between PACIFIDLOG and\n"
	.string "SLATEPORT has a fast-running tide.\p"
	.string "If you decide to SURF, you could end\n"
	.string "up swept away somewhere else.$"

PacifidlogTown_Text_NeatHousesOnWater:
	.string "Our houses float on the water!\n"
	.string "I was born here.\p"
	.string "When the sea rose during the crisis,\n"
	.string "our whole town just… rose with it.\p"
	.string "Turns out floating is a good plan.$"

PacifidlogTown_Text_SkyPillarTooScary:
	.string "The SKY PILLAR? That tall tower out\n"
	.string "past the currents?\p"
	.string "Grandma says when land and sea go to\n"
	.string "war, a green light comes down from\l"
	.string "the top and ends it.\p"
	.string "We don't climb it. We just hope we\n"
	.string "never see that light.$"

PacifidlogTown_Text_TownSign:
	.string "PACIFIDLOG TOWN\p"
	.string "“Where the morning sun smiles upon\n"
	.string "the waters.”$"

PacifidlogTown_Text_FastRunningCurrentAftermath:
	.string "The storm tore two of our rafts\n"
	.string "loose and sent them out to sea.\p"
	.string "Volunteers in AQUA blue helped us\n"
	.string "tie new ones. Grandpa still won't\l"
	.string "let them in the house.$"
