@ Project ARIA — chapter 2 "Mt. Moon, the first tear" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-14).
@ VAR_ARIA_STORY 7 (left Oak's lab) -> tear sealed + Varnell's call = 8 -> Blue at the Route 4 exit = 9.
@ Scenes are queued with VAR_TEMP_9 on map entry (coord triggers fire on VAR_TEMP_9 1).
@ Tears: OBJ_EVENT_GFX_ARIA_TEAR on the bottom-middle tile of its 3x3; the whole 3x3 is solid until sealed
@ (owner 2026-11-14). Mt. Moon's: B2F, old Rocket spot by the crater, (36..38, 20..22).

Aria_Ch0_Return_End::
	end

@ ---- 2.1 Hiker at the Mt. Moon entrance (1F, from Route 4)
Aria_Ch2_Mt1FOnTransition::
	setflag FLAG_TEMP_11
	goto_if_lt VAR_ARIA_STORY, 7, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	goto_if_set FLAG_ARIA_CH2_HIKER_TALKED, Aria_Ch0_Return
	setvar VAR_TEMP_9, 1
	return

Aria_Ch2_HikerStop::
	lockall
	setvar VAR_TEMP_9, 0
	playse SE_PIN
	applymovement LOCALID_MT_MOON_1F_ARIA_HIKER, Common_Movement_ExclamationMark
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch2_Text_Hiker, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_ARIA_CH2_HIKER_TALKED
	releaseall
	end

Aria_Ch2_HikerTalk::
	lock
	faceplayer
	goto_if_set FLAG_ARIA_CH2_HIKER_TALKED, Aria_Ch2_HikerAgain
	msgbox Aria_Ch2_Text_Hiker, MSGBOX_DEFAULT
	setflag FLAG_ARIA_CH2_HIKER_TALKED
	release
	end

Aria_Ch2_HikerAgain::
	msgbox Aria_Ch2_Text_HikerAgain, MSGBOX_DEFAULT
	release
	end

@ ---- 2.2 Camper and Lass (B1F, by the ladder down)
Aria_Ch2_B1FOnTransition::
	setflag FLAG_TEMP_11
	goto_if_lt VAR_ARIA_STORY, 7, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	return

Aria_Ch2_Camper::
	msgbox Aria_Ch2_Text_Camper, MSGBOX_NPC
	end

Aria_Ch2_Lass::
	msgbox Aria_Ch2_Text_Lass, MSGBOX_NPC
	end

@ ---- 2.4 The tear (B2F). The 3x3 is made solid on load.
Aria_Ch2_MtMoonOnLoad::
	goto_if_set FLAG_ARIA_TEAR_MT_MOON_SEALED, Aria_Ch0_Return_End
	setmetatile 36, 20, 0x281, TRUE
	setmetatile 37, 20, 0x281, TRUE
	setmetatile 38, 20, 0x281, TRUE
	setmetatile 36, 21, 0x281, TRUE
	setmetatile 37, 21, 0x281, TRUE
	setmetatile 38, 21, 0x281, TRUE
	setmetatile 36, 22, 0x281, TRUE
	setmetatile 38, 22, 0x281, TRUE
	end

Aria_Ch2_B2FOnTransition::
	goto_if_ne VAR_ARIA_STORY, 7, Aria_Ch0_Return
	goto_if_set FLAG_ARIA_TEAR_MT_MOON_SEALED, Aria_Ch0_Return
	setvar VAR_TEMP_9, 1
	return

@ Talking to the tear does the same as stepping up to it
Aria_Ch2_Tear::
Aria_Ch2_TearScene::
	lockall
	setvar VAR_TEMP_9, 0
	goto_if_ne VAR_ARIA_STORY, 7, Aria_Ch2_TearNotYet
	@ the Rift Scanner beeps faster and faster
	playse SE_POKENAV_CALL
	delay 40
	playse SE_POKENAV_CALL
	delay 24
	playse SE_POKENAV_CALL
	delay 12
	playse SE_POKENAV_CALL
	delay 8
	playse SE_POKENAV_CALL
	delay 30
	@ the room flickers violet
	call Aria_Ch2_Flicker
	msgbox Aria_Ch2_Text_HeldUp, MSGBOX_DEFAULT
	msgbox Aria_Ch2_Text_Read1, MSGBOX_DEFAULT
	call Aria_Ch2_Flicker
	msgbox Aria_Ch2_Text_Read2, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_REFLECT
	fadescreenspeed FADE_TO_WHITE, 4
	setflag FLAG_ARIA_TEAR_MT_MOON_SEALED
	removeobject LOCALID_MT_MOON_B2F_TEAR
	setmetatile 36, 20, 0x281, FALSE
	setmetatile 37, 20, 0x281, FALSE
	setmetatile 38, 20, 0x281, FALSE
	setmetatile 36, 21, 0x281, FALSE
	setmetatile 37, 21, 0x281, FALSE
	setmetatile 38, 21, 0x281, FALSE
	setmetatile 36, 22, 0x281, FALSE
	setmetatile 38, 22, 0x281, FALSE
	special DrawWholeMapView
	fadescreenspeed FADE_FROM_WHITE, 6
	msgbox Aria_Ch2_Text_Seal, MSGBOX_DEFAULT
	closemessage
	delay 60
	@ 2.5 Varnell calls
	playse SE_POKENAV_CALL
	waitse
	delay 20
	playse SE_POKENAV_CALL
	waitse
	setspeaker SP_NAME_VARNELL
	msgbox Aria_Ch2_Text_Varnell, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 8
	releaseall
	end

Aria_Ch2_TearNotYet::
	releaseall
	end

@ quick violet flicker (palette fade to a purple-ish black and back)
Aria_Ch2_Flicker::
	fadescreenspeed FADE_TO_BLACK, 1
	fadescreenspeed FADE_FROM_BLACK, 1
	return

@ ---- 2.6 Blue at the Route 4 exit of Mt. Moon
Aria_Ch2_Route4OnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 8, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_9, 1
	return

Aria_Ch2_BlueScene::
	lockall
	setvar VAR_TEMP_9, 0
	applymovement LOCALID_ROUTE4_ARIA_BLUE, Aria_Ch2_Movement_BlueApproach
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_EAST
	setspeaker SP_NAME_BLUE
	msgbox Aria_Ch2_Text_Blue, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Aria_Ch2_Movement_PlayerStepAside
	applymovement LOCALID_ROUTE4_ARIA_BLUE, Aria_Ch2_Movement_BlueLeave
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_ROUTE4_ARIA_BLUE
	setvar VAR_ARIA_STORY, 9
	releaseall
	end

@ The trigger is (32,6), the only tile below the cave mouth (32,5). Blue (36,7) walks up next to the player.
Aria_Ch2_Movement_BlueApproach::
	walk_up
	walk_left
	walk_left
	walk_left
	step_end

@ He pushes past: the player steps back, Blue walks into the mountain.
Aria_Ch2_Movement_PlayerStepAside::
	lock_facing_direction
	walk_down
	unlock_facing_direction
	face_up
	step_end

Aria_Ch2_Movement_BlueLeave::
	delay_16
	walk_left
	walk_up
	set_invisible
	step_end

Aria_Ch2_Text_Hiker::
	.string "Hey, hold on! You're not heading in\n"
	.string "there alone, are you?\p"
	.string "I've walked these tunnels for twenty\n"
	.string "years. I know every echo in this\l"
	.string "mountain.\p"
	.string "Three nights ago the lights started.\n"
	.string "Down on the lowest floor, by the old\l"
	.string "crater.\p"
	.string "Not torchlight. Not Clefairy. A\n"
	.string "color I don't have a word for.\p"
	.string "My Geodude won't go past the second\n"
	.string "floor any more. It just sits down\l"
	.string "and won't budge.\p"
	.string "...You've got that look. The one\n"
	.string "people get right before they do\l"
	.string "something stupid.\p"
	.string "Fine. If you see it, don't touch it.\n"
	.string "And if the ground starts humming,\l"
	.string "run.$"

Aria_Ch2_Text_HikerAgain::
	.string "If the ground starts humming, run. I\n"
	.string "mean it.$"

Aria_Ch2_Text_Camper::
	.string "There are Pokémon down there that\n"
	.string "aren't from here. I've never seen\l"
	.string "them in any book.\p"
	.string "A week ago they weren't there. Now\n"
	.string "they're just... living here. Like\l"
	.string "they always were.$"

Aria_Ch2_Text_Lass::
	.string "He's been saying that all day. ...I\n"
	.string "believe him, though. I saw one too.$"

Aria_Ch2_Text_Read1::
	.string "...Reading...$"

Aria_Ch2_Text_Read2::
	.string "...Reading...$"

Aria_Ch2_Text_Seal::
	.string "...Seal complete.$"

Aria_Ch2_Text_HeldUp::
	.string "{PLAYER} held up the Rift Scanner.$"

Aria_Ch2_Text_Varnell::
	.string "{PLAYER}! My instruments just went\n"
	.string "quiet. That was you, wasn't it?\p"
	.string "Remarkable. Truly. Do you know how\n"
	.string "long I've spent watching these\l"
	.string "readings from behind a screen?\p"
	.string "And you closed one on your first\n"
	.string "day. Samuel was right about you.\p"
	.string "The data's coming through now.\n"
	.string "It's... more than I hoped. Much\l"
	.string "more.\p"
	.string "Rest a little. Then head on to\n"
	.string "Cerulean. There've been reports from\l"
	.string "the city, and I don't like the sound\l"
	.string "of them.\p"
	.string "Oh, and {PLAYER}... thank you. You\n"
	.string "have no idea how much this means to\l"
	.string "me.$"

Aria_Ch2_Text_Blue::
	.string "......\p"
	.string "So it was you.\p"
	.string "My readings on that crater went flat\n"
	.string "ten minutes ago. A week of data, and\l"
	.string "it just stopped.\p"
	.string "I was going to close it myself.\n"
	.string "Tonight. I had the whole thing\l"
	.string "mapped.\p"
	.string "...Whatever. It's closed. That's\n"
	.string "what matters.\p"
	.string "Did it give you anything? Before it\n"
	.string "went? A pattern, a pulse, anything?\p"
	.string "......\p"
	.string "Figures. Those scanners read\n"
	.string "everything and tell you nothing.\l"
	.string "I've been saying it for months.\p"
	.string "Next time you find one, don't just\n"
	.string "slam it shut. Look at it first.$"

