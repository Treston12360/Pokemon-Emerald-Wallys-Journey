@ Project ARIA — chapter 0 "The call" (docs/KANTO_SCRIPT.md, APPROVED 2026-11-13). Hoenn maps, post-game only.
@ VAR_ARIA_STORY (include/constants/vars_frlg.h, kept in the PC save block):
@   0 = not started   1 = Mom passed on Birch's call   2 = Birch gave the S.S. Anne Ticket
@   3 = Mom's goodbye  4 = Wally said goodbye   5 = on the S.S. Anne (voyage), Oak waits on the Vermilion dock   6 = at Oak's lab (ch. 1 next)
@   7 = ch. 1 done
@ Nothing here runs before the first Champion win (FLAG_SYS_GAME_CLEAR) and Norman's post-game visit
@ (VAR_LITTLEROOT_HOUSES_STATE_BRENDAN 4). Owner 2026-11-14: Norman's visit plays first, the same morning.

@ House 1F OnTransition: queue a scene for the frame script (VAR_TEMP_8). Only in the PLAYER's house
@ (Brendan's house is the rival's house for a female player, and May's for a male player).
Aria_Ch0_BrendansHouseOnTransition::
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Aria_Ch0_HouseOnTransition
	return

Aria_Ch0_MaysHouseOnTransition::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, Aria_Ch0_HouseOnTransition
	return

Aria_Ch0_HouseOnTransition::
	goto_if_unset FLAG_SYS_GAME_CLEAR, Aria_Ch0_Return
	goto_if_lt VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4, Aria_Ch0_Return
	call_if_eq VAR_ARIA_STORY, 0, Aria_Ch0_QueueMomCall
	call_if_eq VAR_ARIA_STORY, 2, Aria_Ch0_QueueMomGoodbye
Aria_Ch0_Return::
	return

Aria_Ch0_QueueMomCall::
	setvar VAR_TEMP_8, 1
	return

Aria_Ch0_QueueMomGoodbye::
	setvar VAR_TEMP_8, 2
	return

@ 0.1 on its own (saves that already had Norman's visit before this update)
Aria_Ch0_MomCallScene::
	lockall
	setvar VAR_TEMP_8, 0
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_ExclamationMark
	waitmovement 0
	call Aria_Ch0_MomCall
	releaseall
	end

@ 0.1 — also called at the end of Norman's visit (players_house.inc)
Aria_Ch0_MomCall::
	setspeaker SP_NAME_MOM
	msgbox Aria_Ch0_Text_MomCall, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 1
	return

@ 0.3 Mom on the way out (after Birch)
Aria_Ch0_MomGoodbyeScene::
	lockall
	setvar VAR_TEMP_8, 0
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_FacePlayer
	waitmovement 0
	setspeaker SP_NAME_MOM
	msgbox Aria_Ch0_Text_MomGoodbye, MSGBOX_DEFAULT
	closemessage
	setvar VAR_ARIA_STORY, 3
	releaseall
	end

@ 0.2 Birch (talking to him in the lab while VAR_ARIA_STORY is 1)
Aria_Ch0_Birch::
	setspeaker SP_NAME_BIRCH
	msgbox Aria_Ch0_Text_Birch1, MSGBOX_DEFAULT
	giveitem ITEM_SS_ANNE_TICKET
	setspeaker SP_NAME_BIRCH
	msgbox Aria_Ch0_Text_Birch2, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 2
	release
	end

Aria_Ch0_BirchAfter::
	setspeaker SP_NAME_BIRCH
	msgbox Aria_Ch0_Text_BirchAfter, MSGBOX_DEFAULT
	release
	end

Aria_Ch0_Text_MomCall:: @ 0.1
	.string "Well, look who's up! Good\n"
	.string "morning, Champion.\p"
	.string "...I don't think I'll ever get tired\n"
	.string "of saying that.\p"
	.string "The phone hasn't stopped ringing,\n"
	.string "you know. Reporters, neighbors,\l"
	.string "people I haven't heard from since we\l"
	.string "left Johto. Everybody wants to say\l"
	.string "they know you.\p"
	.string "Oh! One of those calls was Professor\n"
	.string "Birch.\p"
	.string "He asked you to come over to the lab\n"
	.string "as soon as you're up. He said it\l"
	.string "wasn't urgent...\p"
	.string "But he sounded uneasy. Not like\n"
	.string "himself at all.\p"
	.string "And he called twice. Birch doesn't\n"
	.string "call twice.\p"
	.string "Go on. I'll keep breakfast warm.$"

Aria_Ch0_Text_Birch1:: @ 0.2
	.string "{PLAYER}! Sorry to drag\n"
	.string "you over so early.\p"
	.string "You remember the Pokémon that\n"
	.string "started turning up all over Hoenn?\l"
	.string "The ones that had no business being\l"
	.string "here?\p"
	.string "I've spent months on it. Pokémon\n"
	.string "from regions an ocean away, living\l"
	.string "in our grass like they were born\l"
	.string "there.\p"
	.string "No migration, no shipments, no\n"
	.string "explanation. They're just... here.\p"
	.string "Last night Professor Oak called me\n"
	.string "from Kanto.\p"
	.string "It's started there too. And it isn't\n"
	.string "slowing down, it's getting worse.\l"
	.string "Whole routes changing in weeks.\p"
	.string "Oak has a team looking into it. The\n"
	.string "best researcher in Kanto is leading\l"
	.string "it, apparently.\p"
	.string "And he asked for you. By name.\p"
	.string "He said that you've already stood in\n"
	.string "the middle of one crisis and walked\l"
	.string "out the other side, and he'd rather\l"
	.string "have that on his team than another\l"
	.string "theory.\p"
	.string "......\p"
	.string "I won't pretend I'm thrilled. You\n"
	.string "only just got home.\p"
	.string "But I'd be lying if I said I didn't\n"
	.string "agree with him.$"

Aria_Ch0_Text_Birch2:: @ 0.2
	.string "The S.S. Anne leaves from Slateport.\n"
	.string "Oak will meet you at the dock in\l"
	.string "Vermilion City.\p"
	.string "I'll talk to your mother.\n"
	.string "...Actually, she probably already\l"
	.string "knows. She always does.\p"
	.string "{PLAYER}... be careful, all right?\n"
	.string "Kanto isn't home.$"

Aria_Ch0_Text_BirchAfter:: @ 0.2 repeat (not in script doc: repeats his directions)
	.string "The S.S. Anne leaves\n"
	.string "from Slateport. Oak will meet you at\l"
	.string "the dock in Vermilion City.\p"
	.string "Be careful, {PLAYER}.$"

Aria_Ch0_Text_MomGoodbye:: @ 0.3
	.string "Kanto, hm?\p"
	.string "Your father left for somewhere far\n"
	.string "away once, too. I spent a long time\l"
	.string "waiting by the phone.\p"
	.string "......\p"
	.string "But you're not him, and this isn't\n"
	.string "that. You're going because someone\l"
	.string "needs help.\p"
	.string "Call me when you get there. Every\n"
	.string "once in a while, okay? Even just to\l"
	.string "say you ate something.\p"
	.string "I'm proud of you. Now go, before I\n"
	.string "change my mind and hide your shoes.$"


@ ---- 0.4 / 0.5 Slateport harbour: the S.S. Anne takes the S.S. Tidal's berth while the player is on the way
@ (VAR_ARIA_STORY 2-4). Wally (FLAG_TEMP_12) and the S.S. Anne + its sailor (FLAG_TEMP_11) are hidden otherwise.
Aria_Ch0_HarborOnTransition::
	setflag FLAG_TEMP_11
	setflag FLAG_TEMP_12
	goto_if_lt VAR_ARIA_STORY, 2, Aria_Ch0_Return
	goto_if_gt VAR_ARIA_STORY, 4, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	setflag FLAG_HIDE_SLATEPORT_CITY_HARBOR_SS_TIDAL
	goto_if_eq VAR_ARIA_STORY, 4, Aria_Ch0_Return
	clearflag FLAG_TEMP_12
	return

@ 0.4 Wally
Aria_Ch0_Wally::
	lock
	faceplayer
	goto_if_eq VAR_ARIA_STORY, 4, Aria_Ch0_WallyAfter
	setspeaker SP_NAME_WALLY
	msgbox Aria_Ch0_Text_Wally, MSGBOX_DEFAULT
	setvar VAR_ARIA_STORY, 4
	release
	end

Aria_Ch0_WallyAfter::
	setspeaker SP_NAME_WALLY
	msgbox Aria_Ch0_Text_WallyAfter, MSGBOX_DEFAULT
	release
	end

@ 0.5 Boarding -> 0b Vermilion dock. Needs Wally's goodbye first (VAR_ARIA_STORY 4) and the S.S. Anne Ticket.
Aria_Ch0_AnneSailor::
	lock
	faceplayer
	goto_if_ne VAR_ARIA_STORY, 4, Aria_Ch0_AnneSailorNotYet
	checkitem ITEM_SS_ANNE_TICKET
	goto_if_eq VAR_RESULT, FALSE, Aria_Ch0_AnneSailorNotYet
	msgbox Aria_Ch0_Text_Sailor, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_SHIP
	delay 60
	setvar VAR_ARIA_STORY, 5
	@ Owner 2026-11-14: the voyage is played on the real S.S. Anne (explore, battle its trainers). Walking off
	@ the ship (SSAnne_Exterior) means it has docked in Vermilion: Oak's scene plays there.
	warp MAP_SSANNE_1F_CORRIDOR, 19, 2
	waitstate
	release
	end

Aria_Ch0_AnneSailorNotYet::
	msgbox Aria_Ch0_Text_SailorNotYet, MSGBOX_DEFAULT
	release
	end

@ ---- 0b Vermilion dock (FRLG map SSAnne_Exterior). VAR_ARIA_STORY 5 = on board the S.S. Anne / stepping off.
@ Owner 2026-11-13: Vermilion is NOT marked visited (no Fly there yet); straight to Pallet after a black screen.
Aria_Ch0b_DockOnTransition::
	setflag FLAG_TEMP_11
	goto_if_ne VAR_ARIA_STORY, 5, Aria_Ch0_Return
	clearflag FLAG_TEMP_11
	setvar VAR_TEMP_8, 1
	return

Aria_Ch0b_OakAtDock::
	lockall
	setvar VAR_TEMP_8, 0
	applymovement LOCALID_SSANNE_EXTERIOR_OAK, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_SSANNE_EXTERIOR_OAK, Common_Movement_ExclamationMark
	waitmovement 0
	setspeaker SP_NAME_OAK
	msgbox Aria_Ch0b_Text_Oak, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	delay 60
	setvar VAR_ARIA_STORY, 6
	warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, 10
	waitstate
	releaseall
	end

Aria_Ch0_Text_Wally:: @ 0.4
	.string "Leaving without saying\n"
	.string "goodbye? That's cold, even for you.\p"
	.string "Birch told me. Kanto, huh.\p"
	.string "......\p"
	.string "You know, a year ago I couldn't walk\n"
	.string "up Route 111 without stopping to\l"
	.string "catch my breath.\p"
	.string "Now I'm standing here telling the\n"
	.string "Champion not to worry about the\l"
	.string "Champion's seat.\p"
	.string "Funny how that works.\p"
	.string "Don't worry about the title. I'll\n"
	.string "keep the seat warm. I'm good at\l"
	.string "that.\p"
	.string "...Hey. One more thing.\p"
	.string "Kanto has its own League. Eight\n"
	.string "badges, same as here.\p"
	.string "Everyone over there grew up hearing\n"
	.string "Kanto is where the real trainers\l"
	.string "come from.\p"
	.string "If you're going all that way\n"
	.string "anyway... go take their badges. All\l"
	.string "eight.\p"
	.string "They'll take one look at a trainer\n"
	.string "from Hoenn and think you got lucky.\l"
	.string "That our League is the easy one.\p"
	.string "Don't let them. Prove them wrong.\n"
	.string "Show them what Hoenn is made of.\p"
	.string "I'll keep the seat warm for you. And\n"
	.string "come back. I still owe you a\l"
	.string "rematch.$"

Aria_Ch0_Text_WallyAfter:: @ 0.4 repeat (not in script doc)
	.string "Go on. The S.S. Anne won't\n"
	.string "wait, even for the Champion.$"

Aria_Ch0_Text_Sailor:: @ 0.5
	.string "Sailor: Ticket, please. ...The\n"
	.string "Champion! It's an honor. Welcome\l"
	.string "aboard the S.S. Anne.$"

Aria_Ch0_Text_SailorNotYet:: @ 0.5 before Wally / no ticket (not in script doc)
	.string "Sailor: This is the S.S. Anne, bound\n"
	.string "for Vermilion City in Kanto.\l"
	.string "Boarding is by ticket only.$"

Aria_Ch0b_Text_Oak:: @ 0b.1
	.string "Ah! You must be {PLAYER}!\n"
	.string "Welcome to Kanto!\p"
	.string "I'm Professor Oak. Birch speaks very\n"
	.string "highly of you. More than he does of\l"
	.string "most people, and he's known me for\l"
	.string "thirty years.\p"
	.string "I'm sorry to rush you straight off\n"
	.string "the boat, but I'm afraid there's no\l"
	.string "time to show you the sights.\p"
	.string "My lab is in Pallet Town. There are\n"
	.string "people there you need to meet.$"

