SootopolisCity_House7_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, SootopolisCity_House7_OnFrame
	.byte 0

@ Wally's Journey: this is JUAN's house. Steven brings the player here to
@ watch Wally's title match before the 8th Gym (VAR_WALLY_BROADCAST_STATE 2).
SootopolisCity_House7_OnFrame:
	map_script_2 VAR_WALLY_BROADCAST_STATE, 2, WallyJourney_EventScript_JuansHouseBroadcast
	.2byte 0

SootopolisCity_House7_EventScript_OldMan::
	msgbox SootopolisCity_House7_Text_CityFromEruptedVolcano, MSGBOX_NPC
	end

SootopolisCity_House7_Text_CityFromEruptedVolcano:
	.string "An underwater volcano erupted and\n"
	.string "forced itself up from the depths.\p"
	.string "Its crater emerged from the sea and\n"
	.string "became filled with rainwater.\p"
	.string "That's how SOOTOPOLIS CITY came into\n"
	.string "being.$"

