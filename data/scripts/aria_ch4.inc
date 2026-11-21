@ Project ARIA — chapter 4 "Lavender: the crying in the tower" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-14).
@ VAR_ARIA_STORY 14 -> 15 (Varnell's call on arrival) -> 16 (lens from Fuji) -> 17 (the echo) -> 18 (Poké Flute)
@ -> 19 (Varnell's call: come to Celadon). Fuji is at home (the Rockets are gone). The lens = ITEM_CALIBRATION_LENS
@ (shows the tower's ghosts like FireRed's Silph Scope: battle_main.c / battle_setup.c check either item).

Aria_Ch4_LavenderOnTransition::
	goto_if_lt VAR_ARIA_STORY, 15, Aria_Ch4_LavTrans2
	clearflag FLAG_HIDE_POKEHOUSE_FUJI
Aria_Ch4_LavTrans2::
	call_if_eq VAR_ARIA_STORY, 14, Aria_Ch4_QueueCall
	call_if_eq VAR_ARIA_STORY, 18, Aria_Ch4_QueueCall
	return

Aria_Ch4_QueueCall::
	setvar VAR_TEMP_9, 1
	return

Aria_Ch4_LavenderOnFrame::
	map_script_2 VAR_TEMP_9, 1, Aria_Ch4_LavenderCall
	.2byte 0

Aria_Ch4_LavenderCall::
	lockall
	setvar VAR_TEMP_9, 0
	delay 30
	setspeaker SP_NAME_VARNELL
	goto_if_eq VAR_ARIA_STORY, 18, Aria_Ch4_Call2
	pokenavcall Aria_Ch4_Text_Call1
	waitmessage
	clearflag FLAG_HIDE_POKEHOUSE_FUJI
	setvar VAR_ARIA_STORY, 15
	releaseall
	end

Aria_Ch4_Call2::
	pokenavcall Aria_Ch4_Text_Call2
	waitmessage
	setvar VAR_ARIA_STORY, 19
	releaseall
	end

Aria_Ch4_OldMan::
	msgbox Aria_Ch4_Text_OldMan, MSGBOX_NPC
	end

@ ---- Mr. Fuji (Volunteer House)
Aria_Ch4_Fuji::
	goto_if_ge VAR_ARIA_STORY, 18, Aria_Ch4_FujiAfter
	goto_if_eq VAR_ARIA_STORY, 17, Aria_Ch4_FujiFlute
	goto_if_eq VAR_ARIA_STORY, 16, Aria_Ch4_FujiWait
	setspeaker SP_NAME_FUJI
	msgbox Aria_Ch4_Text_Fuji1, MSGBOX_DEFAULT
	giveitem ITEM_CALIBRATION_LENS
	setspeaker SP_NAME_FUJI
	msgbox Aria_Ch4_Text_Fuji2, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 16
	release
	end

Aria_Ch4_FujiWait::
	setspeaker SP_NAME_FUJI
	msgbox Aria_Ch4_Text_FujiWait, MSGBOX_DEFAULT
	release
	end

Aria_Ch4_FujiFlute::
	setspeaker SP_NAME_FUJI
	msgbox Aria_Ch4_Text_Fuji3, MSGBOX_DEFAULT
	setflag FLAG_GOT_POKE_FLUTE
	giveitem ITEM_POKE_FLUTE
	setvar VAR_ARIA_STORY, 18
	release
	end

Aria_Ch4_FujiAfter::
	setspeaker SP_NAME_FUJI
	msgbox Aria_Ch4_Text_FujiAfter, MSGBOX_DEFAULT
	release
	end

@ ---- Pokémon Tower 7F: the echo (where FireRed had Mr. Fuji and the Rockets)
Aria_Ch4_Tower7FOnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 16, Aria_Ch0_Return_End
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_9, 1
	end

Aria_Ch4_Echo::
	lockall
	setvar VAR_TEMP_9, 0
	goto_if_ne VAR_ARIA_STORY, 16, Aria_Ch4_EchoNot
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch4_Text_Echo1, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PSYBEAM
	hideobjectat LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY, MAP_POKEMON_TOWER_7F
	delay 8
	showobjectat LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY, MAP_POKEMON_TOWER_7F
	delay 8
	hideobjectat LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY, MAP_POKEMON_TOWER_7F
	delay 8
	showobjectat LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY, MAP_POKEMON_TOWER_7F
	msgbox Aria_Ch4_Text_Echo2, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_WEAK
	waitmoncry
	msgbox Aria_Ch4_Text_Aria1, MSGBOX_DEFAULT
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_WEAK
	waitmoncry
	msgbox Aria_Ch4_Text_Aria2, MSGBOX_DEFAULT
	msgbox Aria_Ch4_Text_Sniffle, MSGBOX_DEFAULT
	closemessage
	delay 40
	msgbox Aria_Ch4_Text_Echo3, MSGBOX_DEFAULT
	closemessage
	@ owner 2026-11-20: the echo repeats the last thing it did in the Tower - the cry, then it runs to her,
	@ along the floor beside the LEFT STATUE (x9), then behind its top edge (9,2) - where it goes stays hidden. Ch. 13 shows this same corner from
	@ Aria's side (she is waiting in that nook). Keep this path in sync with the finale.
	applymovement LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY, Aria_Ch4_Movement_EchoRuns
	waitmovement 0
	delay 30
	removeobject LOCALID_POKEMON_TOWER_7F_ARIA_CLEFAIRY
	delay 40
	playse SE_SELECT
	msgbox Aria_Ch4_Text_Echo4, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 17
	releaseall
	end

Aria_Ch4_EchoNot::
	releaseall
	end

Aria_Ch4_Text_Call1::
	.string "… … … … … …\n"
	.string "… … … … … Beep!\p"
	.string "{PLAYER}, you made it through.\n"
	.string "Good. I was worried about that\l"
	.string "tunnel.\p"
	.string "The readings from Pokémon Tower\n"
	.string "have been climbing all week.\l"
	.string "Whatever was crying in Rock\l"
	.string "Tunnel... I think it's coming\l"
	.string "from there.\p"
	.string "I've had a calibration lens\n"
	.string "delivered to the Volunteer\l"
	.string "House, by the tower. The man who\l"
	.string "runs it agreed to hold it for\l"
	.string "you.\p"
	.string "With it, ARIA can read what my\n"
	.string "scanner can't - echoes. Things\l"
	.string "that stayed behind.\p"
	.string "I'd come myself, but... the\n"
	.string "readings need a trainer's\l"
	.string "Pokémon to take them. Mine would\l"
	.string "only get in the way.\p"
	.string "Be gentle in there, {PLAYER}.\n"
	.string "It's a place for the dead.\p"
	.string "… … … … … …\n"
	.string "… … … … … Click!$"

Aria_Ch4_Text_Woman::
	.string "Everyone in Lavender knows someone\n"
	.string "who's buried in that tower.\p"
	.string "Some people say they can hear crying\n"
	.string "up there at night. ...It's been\l"
	.string "worse lately.$"

Aria_Ch4_Text_OldMan:: @ owner-approved resident draft 2026-11-20 (optional clue; locals' belief, not fact)
	.string "I remember when Team Rocket came\n"
	.string "through the tower. Terrible day.\p"
	.string "There was a little girl looking for\n"
	.string "her Clefairy. Someone had taken it,\l"
	.string "so they said.\p"
	.string "They say she stopped all of a\n"
	.string "sudden, like something caught her\l"
	.string "attention. Then part of the ceiling\l"
	.string "came down.\p"
	.string "Nobody ever found out exactly what\n"
	.string "happened. Most figured Rocket\l"
	.string "must've been doing something up\l"
	.string "there.\p"
	.string "They found a man in the rubble too.\n"
	.string "His body was so badly damaged nobody\l"
	.string "could recognize him. Some thought he\l"
	.string "was one of the Rockets.\p"
	.string "Awful thing, what they did to this\n"
	.string "town.$"

Aria_Ch4_Text_LittleGirl::
	.string "Mommy says not to go in the tower.\p"
	.string "She says there's a girl in there\n"
	.string "who's still looking for her friend.$"

Aria_Ch4_Text_Fuji1::
	.string "Ah. You must be the trainer this was\n"
	.string "left for. Please, come in.\p"
	.string "A courier brought it yesterday, from\n"
	.string "some research institute. I'm afraid\l"
	.string "I don't know the people who sent it.\p"
	.string "They were very particular that it\n"
	.string "reach you, and no one else.$"

Aria_Ch4_Text_Fuji2::
	.string "......\p"
	.string "I was in the tower, you know. The\n"
	.string "day Team Rocket came.\p"
	.string "I have spent many years since trying\n"
	.string "to give the Pokémon in this house\l"
	.string "what I couldn't give the ones I lost\l"
	.string "that day.\p"
	.string "There is a man who comes to the\n"
	.string "tower sometimes. Late at night, when\l"
	.string "no one is there.\p"
	.string "I've only ever seen him from my\n"
	.string "window, in the dark. Never his face.\p"
	.string "He stands at a grave with no\n"
	.string "Pokémon's name on it. He never stays\l"
	.string "long.\p"
	.string "I have never gone up to ask whose it\n"
	.string "is. Some grief is not meant to be\l"
	.string "asked about.\p"
	.string "...Forgive an old man. Go on.\n"
	.string "Whatever is crying up there has been\l"
	.string "crying a very long time.$"

Aria_Ch4_Text_FujiWait::
	.string "Whatever is crying up there has been\n"
	.string "crying a very long time. Go gently.$"

Aria_Ch4_Text_Echo1::
	.string "A shape at the end of the room. The\n"
	.string "air is cold.\p"
	.string "The Calibration Lens hums.$"

Aria_Ch4_Text_Echo2::
	.string "The shape becomes a Clefairy, small\n"
	.string "and see-through, curled beside a\l"
	.string "grave.$"

Aria_Ch4_Text_Aria1::
	.string "Aria...?$"

Aria_Ch4_Text_Aria2::
	.string "...Aria...$"

Aria_Ch4_Text_Sniffle::
	.string "...sniffle...$"

Aria_Ch4_Text_Echo3::
	.string "It looks up at you. For a moment it\n"
	.string "looks like it's waiting for someone\l"
	.string "else.\p"
	.string "Then it turns, and runs.$"

Aria_Ch4_Text_Echo4::
	.string "The Rift Scanner reads... then goes\n"
	.string "quiet.$"

Aria_Ch4_Text_Fuji3::
	.string "The crying stopped. I heard it stop,\n"
	.string "all the way down here.\p"
	.string "......\p"
	.string "...Thank you.\p"
	.string "Take this. An old flute. Its song\n"
	.string "wakes even the deepest sleeper.\p"
	.string "I'm too old to go wandering the\n"
	.string "roads with it. You'll find a use for\l"
	.string "it, I think.$"

Aria_Ch4_Text_FujiAfter::
	.string "Has the flute been of use to you? Go\n"
	.string "gently, young one.$"

Aria_Ch4_Text_Call2::
	.string "… … … … … …\n"
	.string "… … … … … Beep!\p"
	.string "{PLAYER}. The lens sent\n"
	.string "everything through.\l"
	.string "...Everything.\p"
	.string "......\p"
	.string "I'd like to see the readings\n"
	.string "with you in person. Come to\l"
	.string "Celadon - my office is in the\l"
	.string "city.\p"
	.string "Blue will be there too; he's\n"
	.string "been tracking the tower for\l"
	.string "weeks.\p"
	.string "… … … … … …\n"
	.string "… … … … … Click!$"

Aria_Ch4_Movement_EchoRuns: @ (11,4) -> along the floor beside the left statue's pedestal -> behind its top edge (9,2)
	walk_fast_left
	walk_fast_up
	walk_fast_up
	walk_fast_left
	step_end
