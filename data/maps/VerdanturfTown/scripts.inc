VerdanturfTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VerdanturfTown_OnTransition
	.byte 0

VerdanturfTown_OnTransition:
	setflag FLAG_VISITED_VERDANTURF_TOWN
	setvar VAR_CONTEST_HALL_STATE, 0
	end

VerdanturfTown_EventScript_Twin::
	lock
	faceplayer
	goto_if_set FLAG_RUSTURF_TUNNEL_OPENED, VerdanturfTown_EventScript_TwinTunnelOpen
	msgbox VerdanturfTown_Text_ManTryingToDigTunnel, MSGBOX_DEFAULT
	applymovement LOCALID_VERDANTURF_TWIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

VerdanturfTown_EventScript_TwinTunnelOpen::
	msgbox VerdanturfTown_Text_ManDugTunnelForLove, MSGBOX_DEFAULT
	applymovement LOCALID_VERDANTURF_TWIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

VerdanturfTown_EventScript_Man::
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, VerdanturfTown_Text_AirCleanHere_W0_Script
	msgbox VerdanturfTown_Text_AirCleanHere, MSGBOX_NPC
	end

VerdanturfTown_Text_AirCleanHere_W0_Script:
	msgbox VerdanturfTown_Text_AirCleanHere_W0, MSGBOX_NPC
	end

VerdanturfTown_EventScript_Camper::
	msgbox VerdanturfTown_Text_MakeBattleTentDebut, MSGBOX_NPC
	end

VerdanturfTown_EventScript_Boy::
	lock
	faceplayer
	goto_if_set FLAG_RUSTURF_TUNNEL_OPENED, VerdanturfTown_EventScript_BoyTunnelOpen
	msgbox VerdanturfTown_Text_GuyTryingToBustThroughCave, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_EventScript_BoyTunnelOpen::
	msgbox VerdanturfTown_Text_EasyToGetToRustboroNow, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_EventScript_TownSign::
	msgbox VerdanturfTown_Text_TownSign, MSGBOX_SIGN
	end

VerdanturfTown_EventScript_WandasHouseSign::
	msgbox VerdanturfTown_Text_WandasHouse, MSGBOX_SIGN
	end

VerdanturfTown_EventScript_BattleTentSign::
	msgbox VerdanturfTown_Text_BattleTentSign, MSGBOX_SIGN
	end

VerdanturfTown_EventScript_RusturfTunnelSign::
	msgbox VerdanturfTown_Text_RusturfTunnelSign, MSGBOX_SIGN
	end

VerdanturfTown_Text_ManTryingToDigTunnel:
	.string "My papa says this tunnel is full of\n"
	.string "timid POKéMON.\p"
	.string "Loud noises scare them. That's why\n"
	.string "people protested the digging.$"

VerdanturfTown_Text_ManDugTunnelForLove:
	.string "A man dug a tunnel for the lady he\n"
	.string "loved.\p"
	.string "I don't really get it. But I kind of\n"
	.string "do.$"

VerdanturfTown_Text_AirCleanHere:
	.string "The winds keep MT. CHIMNEY's ash\n"
	.string "away from VERDANTURF.\p"
	.string "Sick kids from all over HOENN come\n"
	.string "here to breathe. WALLY was one.\p"
	.string "Look at him now.$"

VerdanturfTown_Text_MakeBattleTentDebut:
	.string "I'm on a winning streak, so I'm\n"
	.string "making my BATTLE TENT debut.\p"
	.string "Saw a kid named WALLY train out\n"
	.string "here before he left. Never seen\l"
	.string "anyone take notes like that.$"

VerdanturfTown_Text_GuyTryingToBustThroughCave:
	.string "See the cave by the POKéMON MART?\p"
	.string "A guy in there is breaking boulders\n"
	.string "by hand. Wouldn't use machines.\l"
	.string "Didn't want to scare the WHISMUR.$"

VerdanturfTown_Text_EasyToGetToRustboroNow:
	.string "The cave's a tunnel now! Easy trips\n"
	.string "to RUSTBORO.\p"
	.string "And they did it without bulldozers.\n"
	.string "The WHISMUR are still in there.$"

VerdanturfTown_Text_TownSign:
	.string "VERDANTURF TOWN\p"
	.string "“The windswept highlands with the\n"
	.string "sweet fragrance of grass.”$"

VerdanturfTown_Text_WandasHouse:
	.string "WANDA'S HOUSE$"

VerdanturfTown_Text_BattleTentSign:
	.string "BATTLE TENT VERDANTURF SITE\n"
	.string "“Feast Your Eyes on Battles!”$"

VerdanturfTown_Text_RusturfTunnelSign:
	.string "RUSTURF TUNNEL\n"
	.string "“Linking RUSTBORO and VERDANTURF\p"
	.string "“The tunnel project has been\n"
	.string "canceled.”$"

VerdanturfTown_Text_AirCleanHere_W0:
	.string "WALLY's the CHAMPION. Our WALLY.\p"
	.string "He used to sit on that bench and\n"
	.string "watch the other kids run.\p"
	.string "The whole town cried.$"
