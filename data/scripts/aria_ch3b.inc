@ Project ARIA — chapter 3b "Vermilion, Blue, Rock Tunnel" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-14).
@ VAR_ARIA_STORY 12 -> 13 (Blue at Vermilion's north entrance) -> 14 (Varnell explains Project ARIA, Route 11
@ gate 2F — an old Oak aide spot). Rock Tunnel (needs Flash, owner): the cry builds up, once each
@ (FLAG_ARIA_ROCK_TUNNEL_CRY_1/2). Nobody names the Pokémon: the reveal is in Lavender.

Aria_Ch3b_VermilionOnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 12, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_9, 1
	return

Aria_Ch3b_Blue::
	lockall
	setvar VAR_TEMP_9, 0
	playse SE_PIN
	applymovement LOCALID_VERMILION_ARIA_BLUE, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_VERMILION_ARIA_BLUE, Aria_Ch3b_Movement_BlueUp
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_SOUTH
	setspeaker SP_NAME_BLUE
	msgbox Aria_Ch3b_Text_Blue, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VERMILION_ARIA_BLUE, Aria_Ch3b_Movement_BlueAway
	waitmovement 0
	removeobject LOCALID_VERMILION_ARIA_BLUE
	setvar VAR_ARIA_STORY, 13
	releaseall
	end

Aria_Ch3b_Movement_BlueUp::
	walk_up
	walk_up
	step_end

Aria_Ch3b_Movement_BlueAway::
	walk_down
	walk_down
	walk_down
	walk_down
	set_invisible
	step_end

@ ---- Varnell, Route 11 gate 2F
Aria_Ch3b_GateOnTransition::
	setflag FLAG_TEMP_11
	goto_if_lt VAR_ARIA_STORY, 13, Aria_Ch0_Return
	goto_if_gt VAR_ARIA_STORY, 14, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	return

Aria_Ch3b_Varnell::
	lock
	faceplayer
	setspeaker SP_NAME_VARNELL
	goto_if_eq VAR_ARIA_STORY, 14, Aria_Ch3b_VarnellAgain
	msgbox Aria_Ch3b_Text_Varnell, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 14
	release
	end

Aria_Ch3b_VarnellAgain::
	msgbox Aria_Ch3b_Text_VarnellAgain, MSGBOX_DEFAULT
	release
	end

@ ---- Rock Tunnel: the cry
Aria_Ch3b_B1FOnTransition::
	goto_if_lt VAR_ARIA_STORY, 12, Aria_Ch0_Return
	goto_if_set FLAG_ARIA_ROCK_TUNNEL_CRY_1, Aria_Ch0_Return
	setvar VAR_TEMP_9, 1
	return

Aria_Ch3b_1FOnTransition::
	setflag FLAG_TEMP_11
	goto_if_lt VAR_ARIA_STORY, 12, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	goto_if_set FLAG_ARIA_ROCK_TUNNEL_CRY_2, Aria_Ch0_Return
	goto_if_unset FLAG_ARIA_ROCK_TUNNEL_CRY_1, Aria_Ch0_Return
	setvar VAR_TEMP_8, 1
	return

Aria_Ch3b_Cry1::
	lockall
	setvar VAR_TEMP_9, 0
	setflag FLAG_ARIA_ROCK_TUNNEL_CRY_1
	delay 30
	msgbox Aria_Ch3b_Text_Dots, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_WEAK
	waitmoncry
	msgbox Aria_Ch3b_Text_Echo1, MSGBOX_DEFAULT
	msgbox Aria_Ch3b_Text_Cry1, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Aria_Ch3b_Cry2::
	lockall
	setvar VAR_TEMP_8, 0
	setflag FLAG_ARIA_ROCK_TUNNEL_CRY_2
	delay 30
	msgbox Aria_Ch3b_Text_Dots, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_NORMAL
	waitmoncry
	msgbox Aria_Ch3b_Text_Echo2, MSGBOX_DEFAULT
	msgbox Aria_Ch3b_Text_Cry2, MSGBOX_DEFAULT
	closemessage
	delay 30
	playse SE_SELECT
	msgbox Aria_Ch3b_Text_Beep, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Aria_Ch3b_Hiker::
	msgbox Aria_Ch3b_Text_Hiker, MSGBOX_NPC
	end

Aria_Ch3b_Text_Blue::
	.string "......Oh. It's you.\p"
	.string "You closed the one out by Bill's\n"
	.string "place. I heard. ...Bill told me what\l"
	.string "happened.\p"
	.string "......\p"
	.string "Look. You're working with Varnell\n"
	.string "now, so you should probably know\l"
	.string "something.\p"
	.string "He had a daughter. She was killed in\n"
	.string "the Rocket attacks. Years ago,\l"
	.string "before any of this.\p"
	.string "He doesn't talk about it. Ever. I\n"
	.string "don't even know her name.\p"
	.string "But after that he threw himself into\n"
	.string "his work like it was the only thing\l"
	.string "keeping him standing.\p"
	.string "The tears, the readings, all of it.\n"
	.string "He's taken it further than anyone\l"
	.string "thought was possible.\p"
	.string "......After I lost to Red, I didn't\n"
	.string "have anything either. He's the one\l"
	.string "who took me in.\p"
	.string "Gave me work. Gave me a reason to\n"
	.string "get up in the morning.\p"
	.string "I guess we've been keeping each\n"
	.string "other going.\p"
	.string "...So don't let him down. That's\n"
	.string "all.\p"
	.string "Surge's Gym is in town. He's loud,\n"
	.string "he's tough, and he hates losing more\l"
	.string "than anyone I know.\p"
	.string "You'll like him.$"

Aria_Ch3b_Text_Varnell::
	.string "{PLAYER}! Thank you for coming. I\n"
	.string "wanted to thank you in person - what\l"
	.string "you did at Mt. Moon, and at\l"
	.string "Bill's... I'm sorry that was the one\l"
	.string "you had to close.\p"
	.string "I owe you an explanation. You've\n"
	.string "been sealing tears with my scanner.\l"
	.string "Let me tell you what that data is\l"
	.string "for.\p"
	.string "For years I've been building a\n"
	.string "machine. The only one of its kind.\l"
	.string "It reads the tears, maps where space\l"
	.string "is thinnest, and in time, I believe,\l"
	.string "it will close them for good. All of\l"
	.string "them, at once.\p"
	.string "I call it ARIA. Project ARIA.\n"
	.string "Everything my team does, everything\l"
	.string "you've been doing, feeds into it.\p"
	.string "Every tear you close tells ARIA a\n"
	.string "little more about where the next one\l"
	.string "will open.\p"
	.string "It isn't finished. It needs parts -\n"
	.string "some I can build, some I can't.\l"
	.string "That's where I could use your help.\p"
	.string "Old Rocket sites. Meteorite\n"
	.string "fragments. Things that were never\l"
	.string "meant to be found again.\p"
	.string "I'll tell you more when I know what\n"
	.string "I need. Surge's Gym is just over\l"
	.string "there.\p"
	.string "And {PLAYER}... the road east,\n"
	.string "through Rock Tunnel. My readings\l"
	.string "from that tunnel are strange.\p"
	.string "Be careful in there.$"

Aria_Ch3b_Text_VarnellAgain::
	.string "Rock Tunnel is east, past Route 9.\n"
	.string "Be careful in there, {PLAYER}.$"

Aria_Ch3b_Text_Dots::
	.string "......$"

Aria_Ch3b_Text_Cry1::
	.string "...Ar....$"

Aria_Ch3b_Text_Cry2::
	.string "...Ari....$"

Aria_Ch3b_Text_Echo1::
	.string "A Pokémon's cry echoes through the\n"
	.string "dark.$"

Aria_Ch3b_Text_Echo2::
	.string "Closer now. It sounds like it's\n"
	.string "crying.$"

Aria_Ch3b_Text_Beep::
	.string "The Rift Scanner gives one weak\n"
	.string "beep. Then nothing.$"

Aria_Ch3b_Text_Hiker::
	.string "You heard it too? Don't tell me you\n"
	.string "didn't. That crying.\p"
	.string "I've been through this tunnel a\n"
	.string "hundred times. I know every Pokémon\l"
	.string "that lives in here. That's none of\l"
	.string "them.\p"
	.string "It almost sounds like... words.\p"
	.string "It's coming from the Lavender side.\n"
	.string "Same as the tower. ...I'm not going\l"
	.string "back in there.$"

