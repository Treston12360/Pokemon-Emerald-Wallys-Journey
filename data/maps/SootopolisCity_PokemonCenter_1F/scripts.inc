SootopolisCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SootopolisCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

SootopolisCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_SOOTOPOLIS_CITY
	end

SootopolisCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_SOOTOPOLIS_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

SootopolisCity_PokemonCenter_1F_EventScript_Gentleman::
	lock
	faceplayer
	goto_if_ge VAR_SKY_PILLAR_STATE, 2, SootopolisCity_PokemonCenter_1F_EventScript_GentlemanNoLegendaries
	goto_if_unset FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, SootopolisCity_PokemonCenter_1F_EventScript_GentlemanNoLegendaries
	msgbox SootopolisCity_PokemonCenter_1F_Text_EveryoneTakenRefuge, MSGBOX_DEFAULT
	release
	end

SootopolisCity_PokemonCenter_1F_EventScript_GentlemanNoLegendaries::
	msgbox SootopolisCity_PokemonCenter_1F_Text_WallaceToughestInHoenn, MSGBOX_DEFAULT
	release
	end

SootopolisCity_PokemonCenter_1F_EventScript_Woman::
	lock
	faceplayer
	goto_if_ge VAR_SKY_PILLAR_STATE, 2, SootopolisCity_PokemonCenter_1F_EventScript_WomanNoLegendaries
	goto_if_unset FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, SootopolisCity_PokemonCenter_1F_EventScript_WomanNoLegendaries
	msgbox SootopolisCity_PokemonCenter_1F_Text_ArentPokemonOurFriends, MSGBOX_DEFAULT
	release
	end

SootopolisCity_PokemonCenter_1F_EventScript_WomanNoLegendaries::
	msgbox SootopolisCity_PokemonCenter_1F_Text_AlwaysBeFriendsWithPokemon, MSGBOX_DEFAULT
	release
	end

SootopolisCity_PokemonCenter_1F_Text_WallaceToughestInHoenn:
	.string "This town's GYM LEADER, JUAN, taught\n"
	.string "WALLACE everything he knows.\p"
	.string "And they say WALLACE has been guiding\n"
	.string "a sickly boy from PETALBURG.\p"
	.string "Strength gets passed down here, like\n"
	.string "water from one pool to the next.$"

SootopolisCity_PokemonCenter_1F_Text_EveryoneTakenRefuge:
	.string "We moved everyone we could\n"
	.string "into the lower rooms.\p"
	.string "Some people brought POKéMON.\n"
	.string "Others left everything behind.\p"
	.string "The nurses are sharing food\n"
	.string "and checking every name.$"

SootopolisCity_PokemonCenter_1F_Text_AlwaysBeFriendsWithPokemon:
	.string "People asked if I was afraid\n"
	.string "of my own POKéMON.\p"
	.string "Of course I was frightened.\n"
	.string "But my partner was, too.\p"
	.string "We stayed together.\n"
	.string "That's what friendship means.$"

SootopolisCity_PokemonCenter_1F_Text_ArentPokemonOurFriends:
	.string "How can two POKéMON cause\n"
	.string "so much suffering?\p"
	.string "I know they're frightened\n"
	.string "and confused, too.\p"
	.string "But knowing that doesn't\n"
	.string "make the danger smaller.$"

