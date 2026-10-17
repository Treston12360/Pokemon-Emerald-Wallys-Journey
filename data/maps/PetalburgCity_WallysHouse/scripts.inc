PetalburgCity_WallysHouse_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PetalburgCity_WallysHouse_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PetalburgCity_WallysHouse_OnWarp
	.byte 0

PetalburgCity_WallysHouse_OnWarp:
	map_script_2 VAR_PETALBURG_CITY_STATE, 4, PetalburgCity_WallysHouse_EventScript_PlayerWallysDadFaceEachOther
	.2byte 0

PetalburgCity_WallysHouse_EventScript_PlayerWallysDadFaceEachOther::
	turnobject LOCALID_PLAYER, DIR_EAST
	turnobject LOCALID_WALLYS_HOUSE_WALLYS_DAD, DIR_WEST
	end

PetalburgCity_WallysHouse_OnFrame:
	map_script_2 VAR_PETALBURG_CITY_STATE, 4, PetalburgCity_WallysHouse_EventScript_GiveHMSurf
	.2byte 0

PetalburgCity_WallysHouse_EventScript_GiveHMSurf::
	lockall
	msgbox PetalburgCity_WallysHouse_Text_PleaseExcuseUs, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_HM_SURF
	msgbox PetalburgCity_WallysHouse_Text_NoHmSurf, MSGBOX_DEFAULT
	setvar VAR_PETALBURG_CITY_STATE, 5
	releaseall
	end

PetalburgCity_WallysHouse_EventScript_WallysDad::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, PetalburgCity_WallysHouse_EventScript_DadAfterPlayerChampion
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, PetalburgCity_WallysHouse_EventScript_DefeatedWallyInVictoryRoad
	goto_if_set FLAG_DEFEATED_WALLY_VICTORY_ROAD, PetalburgCity_WallysHouse_EventScript_DefeatedWallyInVictoryRoad
	goto_if_set FLAG_RECEIVED_HM_SURF, PetalburgCity_WallysHouse_EventScript_ReceievedHMSurf
	goto_if_set FLAG_THANKED_FOR_PLAYING_WITH_WALLY, PetalburgCity_WallysHouse_EventScript_PlayedWithWally
	msgbox PetalburgCity_WallysHouse_Text_ThanksForPlayingWithWally, MSGBOX_DEFAULT
	setflag FLAG_THANKED_FOR_PLAYING_WITH_WALLY
	release
	end

PetalburgCity_WallysHouse_EventScript_DadAfterPlayerChampion::
	msgbox PetalburgCity_WallysHouse_Text_DadAfterPlayerChampion, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_MomAfterPlayerChampion::
	msgbox PetalburgCity_WallysHouse_Text_MomAfterPlayerChampion, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_ReceievedHMSurf::
	msgbox PetalburgCity_WallysHouse_Text_WallyIsComingHomeSoon, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_DefeatedWallyInVictoryRoad::
	msgbox PetalburgCity_WallysHouse_Text_YouMetWallyInEverGrandeCity, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_PlayedWithWally::
	msgbox PetalburgCity_WallysHouse_Text_WonderHowWallyIsDoing, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_WallysMom::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, PetalburgCity_WallysHouse_EventScript_MomAfterPlayerChampion
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, PetalburgCity_WallysHouse_EventScript_MomSawBroadcast
	goto_if_set FLAG_RECEIVED_HM_SURF, PetalburgCity_WallysHouse_EventScript_ReceivedHMSurf
	msgbox PetalburgCity_WallysHouse_Text_WallyWasReallyHappy, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_MomSawBroadcast::
	msgbox PetalburgCity_WallysHouse_Text_MomSawBroadcast, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_ReceivedHMSurf::
	msgbox PetalburgCity_WallysHouse_Text_WallyLeftWithoutTelling, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_Text_ThanksForPlayingWithWally:
	.string "You're…\n"
	.string "Ah, you must be {PLAYER}{KUN}, right?\p"
	.string "Thank you for playing with WALLY a\n"
	.string "little while ago.\p"
	.string "He's been frail and sickly ever\n"
	.string "since he was a baby.\p"
	.string "We've sent him to stay with my relatives\n"
	.string "in VERDANTURF TOWN for a while.\p"
	.string "The air is a lot cleaner there\n"
	.string "than it is here.\p"
	.string "What's that? Where's WALLY?\n"
	.string "He's already left, our WALLY.\p"
	.string "I wonder where he could have\n"
	.string "gotten by now?$"

PetalburgCity_WallysHouse_Text_WonderHowWallyIsDoing:
	.string "I wonder how our WALLY is doing?$"

PetalburgCity_WallysHouse_Text_PleaseExcuseUs:
	.string "{PLAYER}{KUN}! Please excuse us for\n"
	.string "dragging you here this way.\p"
	.string "But our WALLY's become very healthy\n"
	.string "since he went to VERDANTURF TOWN.\p"
	.string "We owe it all to you!\p"
	.string "When WALLY left town, you helped\n"
	.string "him catch a POKéMON, right?\p"
	.string "I think that made WALLY really\n"
	.string "happy.\p"
	.string "Actually, not just WALLY.\n"
	.string "It made me, his father, happy too.\p"
	.string "Happy that he's gained such a great\n"
	.string "friend as you.\p"
	.string "This isn't a bribe or anything, but\n"
	.string "I'd really like you to have this.$"

PetalburgCity_WallysHouse_Text_SurfGoAllSortsOfPlaces:
	.string "If your POKéMON can SURF, you'll be\n"
	.string "able to go to all sorts of places.$"

PetalburgCity_WallysHouse_Text_WallyIsComingHomeSoon:
	.string "WALLY calls from the road now.\p"
	.string "He told me he's been challenging\n"
	.string "GYMS. He wouldn't say how many he's\l"
	.string "won. That's how I know it's a lot.\p"
	.string "I'm looking forward to having him\n"
	.string "home again.$"

PetalburgCity_WallysHouse_Text_YouMetWallyInEverGrandeCity:
	.string "We watched it on TV, {PLAYER}{KUN}.\n"
	.string "Our WALLY, beating WALLACE.\p"
	.string "The boy who couldn't run to the end\n"
	.string "of the street without coughing…\p"
	.string "He's the CHAMPION now. I still can't\n"
	.string "say it without my voice shaking.\p"
	.string "He sent word after the match.\n"
	.string "He asked whether you'd reached\l"
	.string "the LEAGUE yet.\p"
	.string "I hope he remembers that winning\n"
	.string "isn't all that matters.$"

PetalburgCity_WallysHouse_Text_WallyWasReallyHappy:
	.string "WALLY was really happy when he told\n"
	.string "us that he caught a POKéMON.\p"
	.string "It's been ages since I've seen him\n"
	.string "smile like that.$"

PetalburgCity_WallysHouse_Text_WallyLeftWithoutTelling:
	.string "I want you to keep this a secret\n"
	.string "from my husband…\p"
	.string "But our WALLY left VERDANTURF TOWN\n"
	.string "without telling anyone.\p"
	.string "He sent a letter. It said, “I need\n"
	.string "to find out what I can do on my own.”\p"
	.string "WALLY is frail, but he's stubborn.\n"
	.string "I'm scared for him. And proud.$"


PetalburgCity_WallysHouse_Text_NoHmSurf:
	.string "You've grown into a fine TRAINER!\p"
	.string "The BALANCE BADGE lets you SURF\n"
	.string "across water. Best of luck!$"


@ Wally's Journey: after the player beats Norman, Steven waits in Wally's house
@ and gives the Mega Bracelet plus the Mega Stone for the chosen starter.
@ Item checks keep the scene safe if the Bag was full the first time.
WallyJourney_EventScript_StevenMegaGift::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_MEGA_BRACELET, WallyJourney_StevenMega_AfterGift
	checkitem ITEM_MEGA_RING
	goto_if_eq VAR_RESULT, TRUE, WallyJourney_StevenMega_CheckStarter
	msgbox WallyJourney_StevenMega_Intro, MSGBOX_DEFAULT
	giveitem ITEM_MEGA_RING
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox WallyJourney_StevenMega_Stone, MSGBOX_DEFAULT

WallyJourney_StevenMega_CheckStarter::
	goto_if_eq VAR_STARTER_MON, 0, WallyJourney_StevenMega_Treecko
	goto_if_eq VAR_STARTER_MON, 1, WallyJourney_StevenMega_Torchic
	goto WallyJourney_StevenMega_Mudkip

WallyJourney_StevenMega_Treecko::
	checkitem ITEM_SCEPTILITE
	goto_if_eq VAR_RESULT, TRUE, WallyJourney_StevenMega_Finish
	giveitem ITEM_SCEPTILITE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	goto WallyJourney_StevenMega_Finish

WallyJourney_StevenMega_Torchic::
	checkitem ITEM_BLAZIKENITE
	goto_if_eq VAR_RESULT, TRUE, WallyJourney_StevenMega_Finish
	giveitem ITEM_BLAZIKENITE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	goto WallyJourney_StevenMega_Finish

WallyJourney_StevenMega_Mudkip::
	checkitem ITEM_SWAMPERTITE
	goto_if_eq VAR_RESULT, TRUE, WallyJourney_StevenMega_Finish
	giveitem ITEM_SWAMPERTITE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
WallyJourney_StevenMega_Finish::
	setflag FLAG_RECEIVED_MEGA_BRACELET
	setflag FLAG_HIDE_WALLYS_HOUSE_STEVEN
	msgbox WallyJourney_StevenMega_End, MSGBOX_DEFAULT
	release
	end

WallyJourney_StevenMega_AfterGift::
	msgbox WallyJourney_StevenMega_AfterText, MSGBOX_DEFAULT
	release
	end

WallyJourney_StevenMega_Intro:
	.string "STEVEN: {PLAYER}! I heard you\n"
	.string "defeated your father, NORMAN.\p"
	.string "When we first met, you were just\n"
	.string "starting out. Look at you now.\p"
	.string "You've grown so much as a TRAINER.\p"
	.string "While I was traveling in another\n"
	.string "region, I came across this.\p"
	.string "I believe it can help you and your\n"
	.string "POKéMON reach new heights.$"

WallyJourney_StevenMega_Stone:
	.string "STEVEN: A MEGA BRACELET is only\n"
	.string "half of it. Your partner needs a\l"
	.string "MEGA STONE that answers its call.\p"
	.string "I found one that suits the POKéMON\n"
	.string "you started your journey with.$"

WallyJourney_StevenMega_End:
	.string "STEVEN: Have your first partner\n"
	.string "hold that stone once it's fully\l"
	.string "evolved.\p"
	.string "When your bond is strong enough,\n"
	.string "the bracelet will respond.\p"
	.string "I look forward to seeing how far\n"
	.string "you'll go. Until next time!$"

WallyJourney_StevenMega_AfterText:
	.string "STEVEN: Your bond with your\n"
	.string "POKéMON is your greatest strength!$"


PetalburgCity_WallysHouse_Text_DadAfterPlayerChampion:
	.string "WALLY called after your battle.\p"
	.string "He told us he'd lost the title,\n"
	.string "and how he'd spoken to you.\p"
	.string "He sounded ashamed. Then he asked\n"
	.string "how we were doing.\p"
	.string "I think he's finding his way back.$"

PetalburgCity_WallysHouse_Text_MomAfterPlayerChampion:
	.string "WALLY told us you won, {PLAYER}.\p"
	.string "He was quiet for a long time.\n"
	.string "Then he started talking about\l"
	.string "the people who helped him grow.\p"
	.string "He's still our determined boy.\n"
	.string "I'm glad he's listening again.$"

PetalburgCity_WallysHouse_Text_MomSawBroadcast:
	.string "We saw the broadcast. All of\n"
	.string "PETALBURG did.\p"
	.string "I used to worry he'd never leave\n"
	.string "his room. Now he's the CHAMPION.\p"
	.string "He still writes home every week.\n"
	.string "He always asks how you're doing.$"
