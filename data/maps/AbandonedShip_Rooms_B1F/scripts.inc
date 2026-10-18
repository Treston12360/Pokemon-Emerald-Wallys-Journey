AbandonedShip_Rooms_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, AbandonedShip_Rooms_B1F_OnResume
	.byte 0

AbandonedShip_Rooms_B1F_OnResume:
	setdivewarp MAP_ABANDONED_SHIP_UNDERWATER2, 17, 4
	end

AbandonedShip_Rooms_B1F_EventScript_FatMan::
	msgbox AbandonedShip_Rooms_B1F_Text_GettingQueasy, MSGBOX_NPC
	end

AbandonedShip_Rooms_B1F_Text_GettingQueasy:
	.string "Urrrrppp…\p"
	.string "It's not even moving, and I'm\n"
	.string "queasy. How did people LIVE on\l"
	.string "these things?\p"
	.string "Then again, how do people live in\n"
	.string "PACIFIDLOG? Same answer, I guess.\l"
	.string "They had no choice.$"

