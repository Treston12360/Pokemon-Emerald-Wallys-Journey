Route116_TunnelersRestHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route116_TunnelersRestHouse_OnTransition
	.byte 0

Route116_TunnelersRestHouse_OnTransition:
	setflag FLAG_LANDMARK_TUNNELERS_REST_HOUSE
	end

Route116_TunnelersRestHouse_EventScript_Tunneler1::
	msgbox Route116_TunnelersRestHouse_Text_WeHadToStopBoring, MSGBOX_NPC
	end

Route116_TunnelersRestHouse_EventScript_Tunneler2::
	msgbox Route116_TunnelersRestHouse_Text_ManDiggingHisWayToVerdanturf, MSGBOX_NPC
	end

Route116_TunnelersRestHouse_EventScript_Tunneler3::
	lock
	faceplayer
	goto_if_set FLAG_RUSTURF_TUNNEL_OPENED, Route116_TunnelersRestHouse_EventScript_TunnelOpened
	msgbox Route116_TunnelersRestHouse_Text_GetToVerdanturfWithoutTunnel, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_EventScript_TunnelOpened::
	msgbox Route116_TunnelersRestHouse_Text_TunnelHasGoneThrough, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_Text_WeHadToStopBoring:
	.string "That Rusturf Tunnel was a Devon\n"
	.string "job. Faster trucks to Verdanturf.\p"
	.string "We had the best drills money could\n"
	.string "buy. Then the Whismur in there\l"
	.string "started going deaf.\p"
	.string "People complained. Devon listened,\n"
	.string "for once. So now we sit here and get\l"
	.string "paid to do nothing.$"

Route116_TunnelersRestHouse_Text_ManDiggingHisWayToVerdanturf:
	.string "There's a man digging to Verdanturf\n"
	.string "by hand. No machines.\p"
	.string "His girlfriend lives on the other\n"
	.string "side. Devon's trucks were never\l"
	.string "coming for him.\p"
	.string "I hope he makes it.$"

Route116_TunnelersRestHouse_Text_GetToVerdanturfWithoutTunnel:
	.string "To get to Verdanturf without using\n"
	.string "this Tunnel, you'd have to cross the\l"
	.string "sea to Dewford, sail on to Slateport,\l"
	.string "then travel through Mauville.$"

Route116_TunnelersRestHouse_Text_TunnelHasGoneThrough:
	.string "Did you hear? The Tunnel went\n"
	.string "through!\p"
	.string "A whole company quit, and one man\n"
	.string "with a shovel finished the job.\p"
	.string "Devon's putting it in their ads now.$"

