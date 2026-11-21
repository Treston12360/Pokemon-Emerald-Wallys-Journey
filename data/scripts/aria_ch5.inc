@ Project ARIA — chapter 5 "Celadon: the office, the report" (docs/KANTO_SCRIPT.md 5.1-5.3, APPROVED 2026-11-14).
@ Map: CeladonCity_VarnellOffice (new, owner approved the building + 13x11 layout 2026-11-21; door at Celadon (45,21),
@ sign "Varnell Research Office" at (46,23) beside the path). Built by tools/aria_celadon_office_map.py.
@ VAR_ARIA_STORY 19 (Varnell's call after the flute) -> entering the office plays 5.1 + 5.2 -> 20.
@ Blue (FLAG_TEMP_11) is only there for the scene; Varnell (FLAG_TEMP_12) is there from 19 on.
@ The drawing (bg events at (8,1)/(9,1), read from (8,2)/(9,2) facing up) is optional, any time (owner 2026-11-21).
@ Blocking (who stands where, who walks) is Claude's first pass; the words are the approved 5.1-5.3 text.
@ DRAFT (not owner-approved wording yet): Aria_Ch5_Text_Drawing.

Aria_Ch5_OfficeOnTransition::
	setflag FLAG_TEMP_11
	setflag FLAG_TEMP_12
	goto_if_lt VAR_ARIA_STORY, 19, Aria_Ch5_Return
	clearflag FLAG_TEMP_12
	goto_if_ne VAR_ARIA_STORY, 19, Aria_Ch5_Return
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_8, 1
Aria_Ch5_Return::
	return

Aria_Ch5_OfficeOnFrame::
	map_script_2 VAR_TEMP_8, 1, Aria_Ch5_Report
	.2byte 0

Aria_Ch5_Report::
	lockall
	setvar VAR_TEMP_8, 0
	delay 20
	@ 5.1 — the receptionist (left of the door, behind her desk)
	turnobject LOCALID_PLAYER, DIR_WEST
	setspeaker SP_NAME_NONE
	msgbox Aria_Ch5_Text_Receptionist, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Aria_Ch5_Movement_PlayerToTable
	waitmovement 0
	turnobject LOCALID_VARNELL_OFFICE_VARNELL, DIR_SOUTH
	turnobject LOCALID_PLAYER, DIR_NORTH
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch5_Text_Var1, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_EAST
	setspeaker SP_NAME_BLUE
	msgbox Aria_Ch5_Text_Blue1, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_NORTH
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch5_Text_Var2, MSGBOX_DEFAULT
	@ 5.2 — the report-back
	msgbox Aria_Ch5_Text_Var3, MSGBOX_DEFAULT
	closemessage
	setspeaker SP_NAME_NONE
	msgbox Aria_Ch5_Text_Player, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_EAST
	setspeaker SP_NAME_BLUE
	msgbox Aria_Ch5_Text_Blue2, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_NORTH
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch5_Text_Var4, MSGBOX_DEFAULT
	msgbox Aria_Ch5_Text_Var5, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_EAST
	setspeaker SP_NAME_BLUE
	msgbox Aria_Ch5_Text_Blue3, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_VARNELL_OFFICE_BLUE, Aria_Ch5_Movement_BlueLeaves
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_VARNELL_OFFICE_BLUE
	delay 20
	turnobject LOCALID_PLAYER, DIR_NORTH
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch5_Text_Var6, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 20
	releaseall
	end

@ player enters on the mat (6,9): up 3, right 1 -> (7,6), the left side of the meeting table
Aria_Ch5_Movement_PlayerToTable:
	walk_up
	walk_up
	walk_up
	walk_right
	step_end

@ Blue (12,6): up 2 past the table, left along row 4 to the walkway, down to the door mat
Aria_Ch5_Movement_BlueLeaves:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

@ ---- talking to people afterwards
Aria_Ch5_Varnell::
	lock
	faceplayer
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch5_Text_VarAfter, MSGBOX_DEFAULT
	release
	end

Aria_Ch5_Receptionist::
	msgbox Aria_Ch5_Text_Receptionist, MSGBOX_NPC
	end

Aria_Ch5_StaffSettings::
	msgbox Aria_Ch5_Text_StaffSettings, MSGBOX_NPC
	end

Aria_Ch5_StaffShipments::
	msgbox Aria_Ch5_Text_StaffShipments, MSGBOX_NPC
	end

Aria_Ch5_StaffDoor::
	msgbox Aria_Ch5_Text_StaffDoor, MSGBOX_NPC
	end

Aria_Ch5_Drawing::
	msgbox Aria_Ch5_Text_Drawing, MSGBOX_SIGN
	end

Aria_Ch5_OfficeSign::
	msgbox Aria_Ch5_Text_OfficeSign, MSGBOX_SIGN
	end

Aria_Ch5_Text_Receptionist::
	.string "Welcome to the Varnell Research\n"
	.string "Office. ...Oh! You're the trainer\l"
	.string "from Hoenn. The Director's expecting\l"
	.string "you. Go on through.$"

Aria_Ch5_Text_Var1::
	.string "{PLAYER}! Welcome, welcome. This is\n"
	.string "the public side of my work - where\l"
	.string "the visitors come, where the grant\l"
	.string "committees sit and nod.\p"
	.string "The real machine is somewhere\n"
	.string "quieter. Somewhere it can't hurt\l"
	.string "anyone if something goes wrong.\p"
	.string "You'll see it one day. When it's\n"
	.string "finished.$"

Aria_Ch5_Text_Blue1::
	.string "...Hey.$"

Aria_Ch5_Text_Var2::
	.string "You know Blue, of course. He's been\n"
	.string "helping me here since - well. For a\l"
	.string "long time now.$"

Aria_Ch5_Text_Var3::
	.string "So. The tower. Tell me everything.$"

Aria_Ch5_Text_Player::
	.string "......$"

Aria_Ch5_Text_Blue2::
	.string "It was weird. It kept saying “Aria.”\n"
	.string "Like the project.\p"
	.string "Maybe the tears pulled it in like\n"
	.string "the other Pokémon that shouldn't be\l"
	.string "here... and it's reacting to the\l"
	.string "machine somehow?$"

Aria_Ch5_Text_Var4::
	.string "......$"

Aria_Ch5_Text_Var5::
	.string "...Perhaps it does know. The tower's\n"
	.string "readings were the highest we've ever\l"
	.string "recorded.\p"
	.string "If that echo was pulled through a\n"
	.string "tear, it may have sensed ARIA\l"
	.string "working - the one thing in Kanto\l"
	.string "that could put it back where it\l"
	.string "belongs. Maybe that's what it was\l"
	.string "asking us to do.\p"
	.string "Good work, both of you.$"

Aria_Ch5_Text_Blue3::
	.string "I'm going to check the readings out\n"
	.string "on Route 16. ...Later.$"

Aria_Ch5_Text_Var6::
	.string "......\p"
	.string "He won't tell you, so I will. When\n"
	.string "he lost to Red, he lost everything\l"
	.string "he thought he was.\p"
	.string "I gave him a desk and something to\n"
	.string "do with his hands. That's all. He\l"
	.string "did the rest.\p"
	.string "He's... he's the closest thing to a\n"
	.string "son I have left.\p"
	.string "...Don't tell him I said that. He'd\n"
	.string "never let me hear the end of it.\p"
	.string "Erika's Gym is just down the street.$"

Aria_Ch5_Text_VarAfter::
	.string "Erika's Gym is just down the street.$"

Aria_Ch5_Text_StaffSettings::
	.string "The Director won't let anyone else\n"
	.string "touch ARIA's settings. Not even me,\l"
	.string "and I built half the calibration\l"
	.string "rig.$"

Aria_Ch5_Text_StaffShipments::
	.string "Shipments go out every week. Where\n"
	.string "to? ...You'd have to ask the\l"
	.string "Director.$"

Aria_Ch5_Text_StaffDoor::
	.string "Sorry, that door's staff only. - No,\n"
	.string "I don't have the key either.$"

Aria_Ch5_Text_Drawing::
	.string "A child's drawing. A girl holding a\n"
	.string "Clefairy.\p"
	.string "In the corner, in crayon: “aria”.$"

Aria_Ch5_Text_OfficeSign::
	.string "Varnell Research Office$"

