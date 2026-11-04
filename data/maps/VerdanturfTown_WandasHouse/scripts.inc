VerdanturfTown_WandasHouse_MapScripts::
	.byte 0

VerdanturfTown_WandasHouse_EventScript_Wally::
	lock
	faceplayer
	@ Wally's Journey: after the Route 111 battle (pending goes back to 0), he says so.
	goto_if_unset FLAG_BADGE03_GET, VerdanturfTown_WandasHouse_EventScript_WallyBeforeRoute111
	goto_if_ne VAR_WALLY_RIVAL_PENDING, 3, VerdanturfTown_WandasHouse_EventScript_WallyAfterRoute111
VerdanturfTown_WandasHouse_EventScript_WallyBeforeRoute111::
	goto_if_set FLAG_WALLY_SPEECH, VerdanturfTown_WandasHouse_EventScript_WallyShortSpeech
	msgbox VerdanturfTown_WandasHouse_Text_StrongerSpeech, MSGBOX_DEFAULT
	setflag FLAG_WALLY_SPEECH
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallyAfterRoute111::
	msgbox VerdanturfTown_WandasHouse_Text_WallyAfterRoute111, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_Text_WallyAfterRoute111:
	.string "Wally: We battled in the Fiery Path,\n"
	.string "didn't we?\p"
	.string "That's how I want to get stronger.\n"
	.string "Battling you, again and again.\p"
	.string "Next time, I'll be ready for\n"
	.string "anything.$"

VerdanturfTown_WandasHouse_EventScript_WallyShortSpeech::
	msgbox VerdanturfTown_WandasHouse_Text_StrongerSpeechShort, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysUncle::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, VerdanturfTown_WandasHouse_EventScript_UncleAfterTitle
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, VerdanturfTown_WandasHouse_EventScript_WallysUncleEverGrande
	goto_if_set FLAG_DEFEATED_WALLY_VICTORY_ROAD, VerdanturfTown_WandasHouse_EventScript_WallysUncleEverGrande
	goto_if_set FLAG_DEFEATED_LAVARIDGE_GYM, VerdanturfTown_WandasHouse_EventScript_WallysUncleSlippedOff
	msgbox VerdanturfTown_WandasHouse_Text_WallysNextDoor, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_UncleAfterPlayerChampion::
	msgbox VerdanturfTown_WandasHouse_Text_UncleAfterPlayerChampion, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_AuntAfterPlayerChampion::
	msgbox VerdanturfTown_WandasHouse_Text_AuntAfterPlayerChampion, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysUncleSlippedOff::
	msgbox VerdanturfTown_WandasHouse_Text_WallySlippedOff, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysUncleEverGrande::
	msgbox VerdanturfTown_WandasHouse_Text_WallyGoneThatFar, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WandasBoyfriend::
	msgbox VerdanturfTown_WandasHouse_Text_CanSeeGirlfriendEveryDay, MSGBOX_NPC
	end

VerdanturfTown_WandasHouse_EventScript_Wanda::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_LAVARIDGE_GYM, VerdanturfTown_WandasHouse_EventScript_WandaDontWorry
	goto_if_set FLAG_DEFEATED_WALLY_MAUVILLE, VerdanturfTown_WandasHouse_EventScript_MeetWanda
	msgbox VerdanturfTown_WandasHouse_Text_DontWorryAboutWally, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_MeetWanda::
	msgbox VerdanturfTown_WandasHouse_Text_MeetWanda, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WandaDontWorry::
	msgbox VerdanturfTown_WandasHouse_Text_DontWorryAboutWally, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysAunt::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, VerdanturfTown_WandasHouse_EventScript_AuntAfterTitle
	goto_if_ge VAR_WALLY_BROADCAST_STATE, 3, VerdanturfTown_WandasHouse_EventScript_WallysAuntEverGrande
	goto_if_set FLAG_DEFEATED_WALLY_VICTORY_ROAD, VerdanturfTown_WandasHouse_EventScript_WallysAuntEverGrande
	goto_if_set FLAG_DEFEATED_LAVARIDGE_GYM, VerdanturfTown_WandasHouse_EventScript_WallysAuntAnythingHappened
	goto_if_set FLAG_RUSTURF_TUNNEL_OPENED, VerdanturfTown_WandasHouse_EventScript_WallysAuntTunnelOpen
	msgbox VerdanturfTown_WandasHouse_Text_DaughtersBoyfriendDriven, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysAuntTunnelOpen::
	msgbox VerdanturfTown_WandasHouse_Text_DaughtersBoyfriendWasDigging, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysAuntAnythingHappened::
	msgbox VerdanturfTown_WandasHouse_Text_IfAnythingHappenedToWally, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_WallysAuntEverGrande::
	msgbox VerdanturfTown_WandasHouse_Text_WallyWasInEverGrande, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_Text_UncleAfterPlayerChampion:
	.string "Wally came by Verdanturf after\n"
	.string "your League battle, {PLAYER}.\p"
	.string "I thought he'd be angry.\n"
	.string "Instead he asked me to walk\l"
	.string "the hill trail with him.\p"
	.string "He didn't race ahead this time.$"

VerdanturfTown_WandasHouse_Text_AuntAfterPlayerChampion:
	.string "His loss was hard to watch.\n"
	.string "We know what he put into it.\p"
	.string "But Wally called afterward.\n"
	.string "He talked about the people who\l"
	.string "helped him get this far.\p"
	.string "I think he'll be all right.$"

VerdanturfTown_WandasHouse_Text_StrongerSpeech:
	.string "Wally: I lost to you, {PLAYER}, but I'm\n"
	.string "not feeling down anymore.\p"
	.string "Because I have a new purpose in life.\n"
	.string "Together with my Ralts, I'm going\l"
	.string "to challenge Pokémon Gyms and become\l"
	.string "a great Trainer.\p"
	.string "Please watch me, {PLAYER}.\n"
	.string "I'm going to be stronger than you.\p"
	.string "When I do, I'm going to challenge you\n"
	.string "to another battle.$"

VerdanturfTown_WandasHouse_Text_StrongerSpeechShort:
	.string "Wally: Please watch me, {PLAYER}.\n"
	.string "I'm going to get stronger than you.\p"
	.string "When I do, I'm going to challenge you\n"
	.string "to another battle.$"

VerdanturfTown_WandasHouse_Text_WallysNextDoor:
	.string "Uncle: Oh! {PLAYER}{KUN}!\n"
	.string "Wally's next door.\p"
	.string "But, boy, there's something I have to\n"
	.string "tell you.\p"
	.string "This natural environment is doing\n"
	.string "wonders for Wally's health.\p"
	.string "Maybe it's not just the environment.\n"
	.string "It could be Pokémon that are giving\l"
	.string "the boy hope.$"

VerdanturfTown_WandasHouse_Text_WallySlippedOff:
	.string "Wally's gone away…\n"
	.string "He slipped off on his own…$"

VerdanturfTown_WandasHouse_Text_WallyGoneThatFar:
	.string "Uncle: You saw the broadcast too?\n"
	.string "Champion! Our Wally!\p"
	.string "I told him he was pushing it, back\n"
	.string "in Mauville. Guess he heard me and\l"
	.string "decided to push harder.\p"
	.string "He is my little brother's son,\n"
	.string "after all.$"

VerdanturfTown_WandasHouse_Text_MeetWanda:
	.string "Wanda: You are?\n"
	.string "Oh, right, I get it!\p"
	.string "You're the {PLAYER} who Wally was\n"
	.string "telling me about.\p"
	.string "I'm Wally's cousin.\n"
	.string "Glad to meet you!\p"
	.string "I think Wally's become a lot more lively\n"
	.string "and healthy since he came here.$"

VerdanturfTown_WandasHouse_Text_DontWorryAboutWally:
	.string "Wanda: Don't worry about Wally.\n"
	.string "He'll be just fine.\p"
	.string "I know my little cousin, and he has\n"
	.string "Pokémon with him, too.$"

VerdanturfTown_WandasHouse_Text_CanSeeGirlfriendEveryDay:
	.string "Thanks to you, I can see my girlfriend\n"
	.string "every day.\l"
	.string "Happy? You bet I am!$"

VerdanturfTown_WandasHouse_Text_DaughtersBoyfriendDriven:
	.string "My daughter's boyfriend is a very\n"
	.string "driven and passionate sort of person.\p"
	.string "He's been digging a tunnel nonstop\n"
	.string "just so he can see my daughter.\p"
	.string "My daughter's a little concerned,\n"
	.string "so she goes out to the tunnel a lot.$"

VerdanturfTown_WandasHouse_Text_DaughtersBoyfriendWasDigging:
	.string "It's amazing. My daughter's boyfriend\n"
	.string "was digging the tunnel by hand!\p"
	.string "It's so incredible!$"

VerdanturfTown_WandasHouse_Text_IfAnythingHappenedToWally:
	.string "If anything were to happen to Wally,\n"
	.string "I would never be able to look his\l"
	.string "parents in Petalburg in the eye…\p"
	.string "He left a note. “Don't worry. I've\n"
	.string "got my team.” That's all he wrote!$"

VerdanturfTown_WandasHouse_Text_WallyWasInEverGrande:
	.string "Wally in Ever Grande? Of course he\n"
	.string "was. He's the Champion!\p"
	.string "We all crammed around the TV for\n"
	.string "the broadcast. My husband cried.\p"
	.string "…Don't tell him I told you.$"

VerdanturfTown_WandasHouse_EventScript_UncleAfterTitle::
	msgbox VerdanturfTown_WandasHouse_Text_UncleAfterTitle, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_EventScript_AuntAfterTitle::
	msgbox VerdanturfTown_WandasHouse_Text_AuntAfterTitle, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_WandasHouse_Text_UncleAfterTitle:
	.string "Uncle: So you're the one who took\n"
	.string "the title from our Wally.\p"
	.string "Don't apologize. The boy needed\n"
	.string "someone to knock him back down to\l"
	.string "earth. Better a friend than a fall.$"

VerdanturfTown_WandasHouse_Text_AuntAfterTitle:
	.string "Wally stayed with us a few days\n"
	.string "after he lost the title.\p"
	.string "He helped the kids with their\n"
	.string "breathing exercises. Patiently.\p"
	.string "He used to be one of them.\n"
	.string "I think he finally remembered.$"
