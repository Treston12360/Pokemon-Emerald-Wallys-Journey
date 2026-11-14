@ Project ARIA — chapter 3 "The man from the fire" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-14).
@ VAR_ARIA_STORY 9 (after Blue at Mt. Moon) -> 10 officer in Cerulean -> 11 Bill -> 12 tear sealed + Varnell.
@ Route 25 tear: the open grass by the water east of Bill's garden, (56..58, 5..7); Kiyoshi stands at (55,8).
@ Kiyoshi: TRAINER_SAGE_KIYOSHI, weak on purpose (owner), not in level scaling. Story lines come after the
@ battle on the map (owner rule: never only in the skippable defeat text).

@ ---- 3.1 Officer at Cerulean's north exit (Nugget Bridge)
Aria_Ch3_CeruleanOnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 9, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_9, 1
	return

Aria_Ch3_OfficerStop::
	lockall
	setvar VAR_TEMP_9, 0
	playse SE_PIN
	applymovement LOCALID_CERULEAN_ARIA_OFFICER, Common_Movement_ExclamationMark
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch3_Text_Officer, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 10
	releaseall
	end

Aria_Ch3_OfficerTalk::
	lock
	faceplayer
	goto_if_ge VAR_ARIA_STORY, 10, Aria_Ch3_OfficerAgain
	msgbox Aria_Ch3_Text_Officer, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 10
	release
	end

Aria_Ch3_OfficerAgain::
	msgbox Aria_Ch3_Text_OfficerAgain, MSGBOX_DEFAULT
	release
	end

@ ---- Route 25: Bill outside the Sea Cottage, Kiyoshi and the tear by the water
Aria_Ch3_Route25OnLoad::
	goto_if_lt VAR_ARIA_STORY, 10, Aria_Ch0_Return_End
	goto_if_set FLAG_ARIA_TEAR_ROUTE25_SEALED, Aria_Ch0_Return_End
	setmetatile 56, 5, 0x009, TRUE
	setmetatile 57, 5, 0x008, TRUE
	setmetatile 58, 5, 0x009, TRUE
	setmetatile 56, 6, 0x001, TRUE
	setmetatile 57, 6, 0x008, TRUE
	setmetatile 58, 6, 0x001, TRUE
	setmetatile 56, 7, 0x009, TRUE
	setmetatile 58, 7, 0x009, TRUE
	end

Aria_Ch3_Route25OnTransition::
	setflag FLAG_TEMP_11
	setflag FLAG_TEMP_12
	goto_if_lt VAR_ARIA_STORY, 10, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	goto_if_set FLAG_ARIA_TEAR_ROUTE25_SEALED, Aria_Ch0_Return
	clearflag FLAG_TEMP_12
	setvar VAR_TEMP_9, 1
	return

@ 3.2 Bill
Aria_Ch3_Bill::
	lock
	faceplayer
	goto_if_set FLAG_ARIA_TEAR_ROUTE25_SEALED, Aria_Ch3_BillLater
	goto_if_ge VAR_ARIA_STORY, 11, Aria_Ch3_BillAgain
	setspeaker SP_NAME_BILL
	msgbox Aria_Ch3_Text_Bill, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 11
	release
	end

Aria_Ch3_BillAgain::
	setspeaker SP_NAME_BILL
	msgbox Aria_Ch3_Text_BillAgain, MSGBOX_DEFAULT
	release
	end

Aria_Ch3_BillLater::
	setspeaker SP_NAME_BILL
	msgbox Aria_Ch3_Text_BillLater, MSGBOX_DEFAULT
	release
	end

@ 3.3 / 3.4 Kiyoshi and the tear (stepping past x=53, or talking to him / the tear)
Aria_Ch3_Kiyoshi::
Aria_Ch3_KiyoshiStop::
	lockall
	setvar VAR_TEMP_9, 0
	goto_if_set FLAG_ARIA_CH3_KIYOSHI_BATTLED, Aria_Ch3_CloseTear
	playse SE_PIN
	applymovement LOCALID_ROUTE25_ARIA_KIYOSHI, Common_Movement_ExclamationMark
	waitmovement 0
	msgbox Aria_Ch3_Text_KiyoshiIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_SAGE_KIYOSHI, Aria_Ch3_Text_KiyoshiDefeat
	setflag FLAG_ARIA_CH3_KIYOSHI_BATTLED
	setspeaker SP_NAME_KIYOSHI
	msgbox Aria_Ch3_Text_KiyoshiStory, MSGBOX_DEFAULT
	closemessage
	delay 30
Aria_Ch3_CloseTear::
	@ the Rift Scanner starts beeping
	playse SE_SELECT @ the Rift Scanner beeping
	delay 30
	playse SE_SELECT @ the Rift Scanner beeping
	delay 20
	setspeaker SP_NAME_KIYOSHI
	msgbox Aria_Ch3_Text_KiyoshiPlead, MSGBOX_DEFAULT
	closemessage
	@ Varnell calls
	setspeaker SP_NAME_VARNELL
	pokenavcall Aria_Ch3_Text_Varnell1
	waitmessage
	delay 20
	delay 30
	msgbox Aria_Ch3_Text_Silence, MSGBOX_DEFAULT
	msgbox Aria_Ch3_Text_HeldUp, MSGBOX_DEFAULT
	setspeaker SP_NAME_KIYOSHI
	msgbox Aria_Ch3_Text_KiyoshiNo, MSGBOX_DEFAULT
	msgbox Aria_Ch3_Text_Reading, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 3
	setspeaker SP_NAME_KIYOSHI
	message Aria_Ch3_Text_Burns
	waitmessage
	special ShakeCamera
	waitstate
	waitbuttonpress
	closemessage
	playse SE_M_REFLECT
	fadescreenspeed FADE_TO_WHITE, 4
	setflag FLAG_ARIA_TEAR_ROUTE25_SEALED
	removeobject LOCALID_ROUTE25_ARIA_KIYOSHI
	removeobject LOCALID_ROUTE25_ARIA_TEAR
	setmetatile 56, 5, 0x009, FALSE
	setmetatile 57, 5, 0x008, FALSE
	setmetatile 58, 5, 0x009, FALSE
	setmetatile 56, 6, 0x001, FALSE
	setmetatile 57, 6, 0x008, FALSE
	setmetatile 58, 6, 0x001, FALSE
	setmetatile 56, 7, 0x009, FALSE
	setmetatile 58, 7, 0x009, FALSE
	special DrawWholeMapView
	delay 30
	fadescreenspeed FADE_FROM_WHITE, 8
	delay 90
	@ 3.5 Bill
	setspeaker SP_NAME_BILL
	msgbox Aria_Ch3_Text_BillAfter, MSGBOX_DEFAULT
	closemessage
	delay 40
	@ 3.6 Varnell calls again
	setspeaker SP_NAME_VARNELL
	pokenavcall Aria_Ch3_Text_Varnell2
	waitmessage
	delay 20
	setvar VAR_ARIA_STORY, 12
	releaseall
	end

Aria_Ch3_Text_Officer::
	.string "You're the trainer from Hoenn,\n"
	.string "right? Professor Oak said you might\l"
	.string "come this way.\p"
	.string "We've had calls all night from up\n"
	.string "the cape. Smoke, but no fire.\l"
	.string "Somebody screaming in a language\l"
	.string "nobody can place. People are calling\l"
	.string "it a ghost.\p"
	.string "Bill, the PC guy, lives out at the\n"
	.string "end of Route 25. He's the one who\l"
	.string "called it in.\p"
	.string "I'd go myself, but... look, I've got\n"
	.string "a city to keep calm. Can you check\l"
	.string "on him?$"

Aria_Ch3_Text_OfficerAgain::
	.string "Bill's place is at the end of Route\n"
	.string "25, past Nugget Bridge. Please,\l"
	.string "check on him.$"

Aria_Ch3_Text_Bill::
	.string "Oh, thank goodness. You're the one\n"
	.string "Oak sent? Then you know about the\l"
	.string "tears.\p"
	.string "I know what it feels like to get\n"
	.string "pulled through something you don't\l"
	.string "understand. Trust me.\p"
	.string "But this... this is different.\n"
	.string "There's a man out by the cape. He's\l"
	.string "not hurt. He's... wrong.\p"
	.string "His clothes are burned. He smells\n"
	.string "like smoke. And he keeps asking\l"
	.string "where the tower went.\p"
	.string "He set his Pokémon on me when I got\n"
	.string "close. Old Pokémon. I mean old - the\l"
	.string "way they look, the way they move.\l"
	.string "Like something out of my\l"
	.string "grandfather's books.\p"
	.string "Please. Be careful.$"

Aria_Ch3_Text_BillAgain::
	.string "He's out past the garden, by the\n"
	.string "water. Please be careful.$"

Aria_Ch3_Text_KiyoshiIntro::
	.string "Stay back! Stay back, I said!\p"
	.string "Where is the tower? Where are the\n"
	.string "bells? Who are you?!$"

Aria_Ch3_Text_KiyoshiDefeat::
	.string "......$"

Aria_Ch3_Text_KiyoshiStory::
	.string "......\p"
	.string "Your Pokémon... they are not afraid\n"
	.string "of fire.\p"
	.string "Forgive me. I am Sage Kiyoshi, of\n"
	.string "the Brass Tower of Ecruteak.\p"
	.string "One moment the tower was on fire.\n"
	.string "The roof was coming down. I was\l"
	.string "burning.\p"
	.string "I heard the three of them crying in\n"
	.string "the flames, and I could not reach\l"
	.string "them.\p"
	.string "And the next moment... I was here.\n"
	.string "The air is cold. The sea is wrong.\l"
	.string "Nothing is burning.\p"
	.string "What is this place? What year is\n"
	.string "this?\p"
	.string "......\p"
	.string "Then everyone I know is gone. And\n"
	.string "still the tower burns, somewhere\l"
	.string "behind that light.$"

Aria_Ch3_Text_KiyoshiPlead::
	.string "What is that? What are you doing?\p"
	.string "No. No, no - please. If you close\n"
	.string "it, I go back. Back into the fire.\p"
	.string "I can feel it pulling at me. I can\n"
	.string "still feel the heat. Please. Let me\l"
	.string "stay.$"

Aria_Ch3_Text_Varnell1:: @ PokéNav call (owner 2026-11-14: Varnell talks to you through the PokéNav)
	.string "… … … … … …\n"
	.string "… … … … … Beep!\p"
	.string "{PLAYER}, I'm seeing\n"
	.string "your readings. Listen to me very\l"
	.string "carefully.\p"
	.string "I know what he's asking. I know\n"
	.string "how it sounds.\p"
	.string "But he isn't supposed to be\n"
	.string "here. If he stays, everything he\l"
	.string "would have done, everything that\l"
	.string "came after him... We cannot\l"
	.string "change time. Not even for one\l"
	.string "man. It could be catastrophic.\p"
	.string "Close it. Please.\p"
	.string "… … … … … …\n"
	.string "… … … … … Click!$"

Aria_Ch3_Text_Silence::
	.string "......$"

Aria_Ch3_Text_HeldUp::
	.string "{PLAYER} held up the Rift Scanner.$"

Aria_Ch3_Text_KiyoshiNo::
	.string "No... please...$"

Aria_Ch3_Text_Reading::
	.string "...Reading...$"

Aria_Ch3_Text_Burns::
	.string "It burns! IT BURNS! AHHHHH!.....$"

Aria_Ch3_Text_BillAfter::
	.string "......\p"
	.string "He knew. The whole time, he knew\n"
	.string "where he was going back to.\p"
	.string "I've spent my whole life building\n"
	.string "machines that move things from one\l"
	.string "place to another.\p"
	.string "I never once thought about what it's\n"
	.string "like to be the thing that gets\l"
	.string "moved.\p"
	.string "...Go on. I need to sit down for a\n"
	.string "while.$"

Aria_Ch3_Text_Varnell2:: @ PokéNav call (owner 2026-11-14: Varnell talks to you through the PokéNav)
	.string "… … … … … …\n"
	.string "… … … … … Beep!\p"
	.string "{PLAYER}. Are you all right?\p"
	.string "I'm sorry you had to be the one\n"
	.string "to do that. I mean that.\p"
	.string "......\p"
	.string "But do you understand what we\n"
	.string "just saw? A living person.\l"
	.string "Carried a hundred and fifty\l"
	.string "years, whole. Breathing.\l"
	.string "Speaking. Himself.\p"
	.string "I've read every theory there is,\n"
	.string "and every one of them said it\l"
	.string "couldn't be done.\p"
	.string "...Forgive me. A man suffered\n"
	.string "tonight, and here I am talking\l"
	.string "about theories. It's been a long\l"
	.string "night.\p"
	.string "Misty's Gym is in Cerulean, if\n"
	.string "you're ready for it. I'll be in\l"
	.string "touch.\p"
	.string "… … … … … …\n"
	.string "… … … … … Click!$"

Aria_Ch3_Text_BillLater::
	.string "I keep hearing it. That scream.\n"
	.string "...Go on. I'll be all right.$"

