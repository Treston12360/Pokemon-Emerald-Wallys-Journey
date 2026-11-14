@ Project ARIA — chapter 1 "The investigation team" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-13).
@ VAR_ARIA_STORY 6 (arrived from the Vermilion dock) -> scene -> 7. Varnell uses an existing scientist sprite
@ until his own is made (owner 2026-11-14). Varnell and Blue (FLAG_TEMP_11) are only there for the scene.
Aria_Ch1_LabOnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 6, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	clearflag FLAG_HIDE_OAK_IN_HIS_LAB
	setobjectxyperm LOCALID_OAKS_LAB_PROF_OAK, 6, 9
	setobjectmovementtype LOCALID_OAKS_LAB_PROF_OAK, MOVEMENT_TYPE_FACE_DOWN
	setvar VAR_TEMP_8, 1
	return

Aria_Ch1_Lab::
	lockall
	setvar VAR_TEMP_8, 0
	turnobject LOCALID_PLAYER, DIR_NORTH
	delay 30
	msgbox Aria_Ch1_Text_Oak1, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_EAST
	msgbox Aria_Ch1_Text_Var1, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch1_Text_Oak2, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_WEST
	msgbox Aria_Ch1_Text_Blue1, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch1_Text_Oak3, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_WEST
	msgbox Aria_Ch1_Text_Blue2, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OAKS_LAB_ARIA_BLUE, Aria_Ch1_Movement_BlueLeaves
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_OAKS_LAB_ARIA_BLUE
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch1_Text_Oak4, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_EAST
	msgbox Aria_Ch1_Text_Var2, MSGBOX_DEFAULT
	giveitem ITEM_RIFT_SCANNER
	msgbox Aria_Ch1_Text_Var3, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Aria_Ch1_Text_Oak5, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 7
	releaseall
	end

@ Talking to Oak after ch. 1 (repeats his directions; not a script-doc line — S-TALK will write Oak's own)
Aria_Ch1_OakAfter::
	msgbox Aria_Ch1_Text_OakAfter, MSGBOX_DEFAULT
	release
	end

@ Blue (4,10): down 1, right 1, down 1 onto the door (5,12)
Aria_Ch1_Movement_BlueLeaves::
	walk_down
	walk_right
	walk_down
	set_invisible
	step_end

Aria_Ch1_Text_Oak1::
	.string "Oak: {PLAYER}, allow me to introduce\n"
	.string "Dr. Elias Varnell.\p"
	.string "If you've read anything about space-\n"
	.string "time research in the last twenty\l"
	.string "years, you've read his work.\p"
	.string "He is the finest researcher in\n"
	.string "Kanto. I'd say the world, but he'd\l"
	.string "only argue.$"

Aria_Ch1_Text_Var1::
	.string "Varnell: You flatter me, Samuel. And\n"
	.string "you exaggerate.\p"
	.string "{PLAYER}. It's a real pleasure.\p"
	.string "I read every report from Hoenn.\n"
	.string "Groudon. Kyogre. Rayquaza, brought\l"
	.string "down from the sky.\p"
	.string "And in every report, the same name,\n"
	.string "right in the middle of it.\p"
	.string "Legendary Pokémon. Forces that shape\n"
	.string "the land and the sea and the sky.\p"
	.string "I've spent my whole life studying\n"
	.string "Pokémon like that. Their traces,\l"
	.string "their old records, the marks they\l"
	.string "leave in space and time. Always from\l"
	.string "a distance. From behind the glass of\l"
	.string "an instrument.\p"
	.string "You stood right in front of them.\n"
	.string "And you're still here.\p"
	.string "I'm leading the investigation into\n"
	.string "these anomalies. I'd be very glad to\l"
	.string "have you with us.$"

Aria_Ch1_Text_Oak2::
	.string "Oak: And this is my grandson, Blue.\n"
	.string "He's been working with Dr. Varnell\l"
	.string "for... how long now?$"

Aria_Ch1_Text_Blue1::
	.string "Blue: ......\p"
	.string "Long enough.\p"
	.string "So this is the Champion you shipped\n"
	.string "in from Hoenn.\p"
	.string "Gramps, we had this handled. I've\n"
	.string "been mapping these things for\l"
	.string "months.$"

Aria_Ch1_Text_Oak3::
	.string "Oak: Blue, nobody is saying you\n"
	.string "haven't-$"

Aria_Ch1_Text_Blue2::
	.string "Blue: Then why is there a stranger\n"
	.string "standing in the lab?\p"
	.string "......\p"
	.string "Whatever. Do what you want.\p"
	.string "Just stay out of my way, Champion.$"

Aria_Ch1_Text_Oak4::
	.string "Oak: ...Forgive him. He hasn't had\n"
	.string "an easy few years.$"

Aria_Ch1_Text_Var2::
	.string "Varnell: He'll come around. Blue's a\n"
	.string "good young man. He just doesn't like\l"
	.string "sharing his work.\p"
	.string "I suppose I taught him that.\p"
	.string "Now. Down to business.\p"
	.string "We call them anomalies, but really\n"
	.string "they're tears. Small breaks in\l"
	.string "space, where something from\l"
	.string "somewhere else slips through.\p"
	.string "Something is happening inside Mt.\n"
	.string "Moon. Our instruments call it a\l"
	.string "“meteorite anomaly”. The hikers call\l"
	.string "it a light in the tunnels that won't\l"
	.string "stay in one place.\p"
	.string "I'd like you to look into it.\p"
	.string "I built this for exactly this kind\n"
	.string "of work.$"

Aria_Ch1_Text_Var3::
	.string "Varnell: If you find a tear, point\n"
	.string "the scanner at it. It will read it,\l"
	.string "and then seal it.\p"
	.string "Every tear we close tells us a\n"
	.string "little more about what's causing\l"
	.string "them.\p"
	.string "Oh, and one practical matter.\p"
	.string "Since the Rocket years, the League\n"
	.string "has locked down anywhere that could\l"
	.string "be dangerous. Only licensed Kanto\l"
	.string "trainers get in. The same goes for\l"
	.string "field moves: your Hoenn licenses\l"
	.string "don't count on this side of the sea.\p"
	.string "So I've taken the liberty of\n"
	.string "registering you for the Kanto Gym\l"
	.string "challenge.\p"
	.string "Consider it my thanks in advance.$"

Aria_Ch1_Text_Oak5::
	.string "Oak: Pewter City is on the way to\n"
	.string "Mt. Moon. Brock's Gym is there.\p"
	.string "He'll give you a proper Kanto\n"
	.string "welcome, I'm sure.\p"
	.string "Good luck, {PLAYER}. And thank you\n"
	.string "for coming all this way.$"

Aria_Ch1_Text_OakAfter::
	.string "Oak: Pewter City is on the way to\n"
	.string "Mt. Moon. Brock's Gym is there.\p"
	.string "Good luck, {PLAYER}.$"

