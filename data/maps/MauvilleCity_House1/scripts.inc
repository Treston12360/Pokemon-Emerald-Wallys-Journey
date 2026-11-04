MauvilleCity_House1_MapScripts::
	.byte 0

MauvilleCity_House1_EventScript_RockSmashDude::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_HM_ROCK_SMASH, MauvilleCity_House1_EventScript_ReceivedRockSmash
	msgbox MauvilleCity_House1_Text_NoHMChat, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_HM_ROCK_SMASH
	setflag FLAG_HIDE_ROUTE_111_ROCK_SMASH_TIP_GUY
	msgbox MauvilleCity_House1_Text_NoHMAdvice, MSGBOX_DEFAULT
	release
	end

MauvilleCity_House1_EventScript_ReceivedRockSmash::
	msgbox MauvilleCity_House1_Text_MonCanFlyOutOfSmashedRock, MSGBOX_DEFAULT
	release
	end

MauvilleCity_House1_Text_ImRockSmashDudeTakeThis:
	.string "Woohoo!\p"
	.string "I hear people call me the Rock Smash\n"
	.string "Guy, but I find that sort of degrading.\p"
	.string "I think I deserve a bit more respect,\n"
	.string "like maybe the Rock Smash Dude.\p"
	.string "Woohoo!\p"
	.string "Anyways, your Pokémon look pretty\n"
	.string "strong.\p"
	.string "I like that!\n"
	.string "Here, take this Hidden Machine!$"

MauvilleCity_House1_Text_ExplainRockSmash:
	.string "That HM contains Rock Smash.\p"
	.string "If you come across large boulders\n"
	.string "that block your path…\p"
	.string "Well, use that HM move and smash\n"
	.string "them right out of your way!\p"
	.string "Yes, sir! Smash rocks aside, I say!\n"
	.string "Woohoo!$"

MauvilleCity_House1_Text_MonCanFlyOutOfSmashedRock:
	.string "Oh, yes, if you smash a rock, a Pokémon\n"
	.string "could come flying out of hiding.\p"
	.string "Woohoo!$"


MauvilleCity_House1_Text_NoHMChat:
	.string "Woohoo! I'm the Rock Smash Dude!\p"
	.string "Your team looks strong enough to\n"
	.string "smash any challenge!$"

MauvilleCity_House1_Text_NoHMAdvice:
	.string "The Dynamo Badge lets you smash\n"
	.string "cracked rocks along the road!$"
