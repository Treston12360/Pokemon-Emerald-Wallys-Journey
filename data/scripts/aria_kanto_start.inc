@ Project ARIA (S0): the state Kanto starts in. Run once per save (SaveBlockPC_InitKanto, src/event_data.c):
@ a new game, or an older save the first time it loads with Kanto in the game.
@ ARIA is set AFTER FireRed's story: Kanto's own FireRed scenes are not played. FireRed's new-game hidden
@ people first (EventScript_ResetAllMapFlagsFrlg), then FireRed's story scenes the player won't play are set
@ as already over or hidden. Every line here is listed in docs/KANTO_S0_STRIPPED.md with its reason.
EventScript_AriaKantoStart::
	call EventScript_ResetAllMapFlagsFrlg
	@ Oak's lab: no starter Poké Balls or Pokédexes on the table (the player already has a team and a Pokédex);
	@ Blue is placed by chapter 1's script, not FireRed's
	setflag FLAG_HIDE_BULBASAUR_BALL
	setflag FLAG_HIDE_SQUIRTLE_BALL
	setflag FLAG_HIDE_CHARMANDER_BALL
	setflag FLAG_HIDE_POKEDEX
	setflag FLAG_HIDE_RIVAL_IN_LAB
	@ Viridian: the old man is up and about (FireRed's "after the parcel" state), not lying in the road
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2
	end

@ Step 2. Owner 2026-11-14: remove all of FireRed's Team Rocket and Blue rival scenes from Kanto.
@ (Viridian Gym's Giovanni and the Champion's room are NOT touched: owner to decide.)
EventScript_AriaKantoStart2::
	setflag FLAG_HIDE_CELADON_ROCKETS
	setflag FLAG_HIDE_CERULEAN_RIVAL
	setflag FLAG_HIDE_CERULEAN_ROCKET
	setflag FLAG_HIDE_CREDITS_RIVAL
	setflag FLAG_HIDE_FIVE_ISLAND_ROCKETS
	setflag FLAG_HIDE_FOUR_ISLAND_RIVAL
	setflag FLAG_HIDE_GAME_CORNER_ROCKET
	setflag FLAG_HIDE_HIDEOUT_GIOVANNI
	setflag FLAG_HIDE_ICEFALL_CAVE_ROCKETS
	setflag FLAG_HIDE_MISC_KANTO_ROCKETS
	setflag FLAG_HIDE_MT_EMBER_EXTERIOR_ROCKETS
	setflag FLAG_HIDE_ROUTE_22_RIVAL
	setflag FLAG_HIDE_SAFFRON_ROCKETS
	setflag FLAG_HIDE_SILPH_RIVAL
	setflag FLAG_HIDE_SILPH_ROCKETS
	setflag FLAG_HIDE_SIX_ISLAND_POKECENTER_RIVAL
	setflag FLAG_HIDE_SS_ANNE_RIVAL
	setflag FLAG_HIDE_TOWER_RIVAL
	setflag FLAG_HIDE_TOWER_ROCKET_1
	setflag FLAG_HIDE_TOWER_ROCKET_2
	setflag FLAG_HIDE_TOWER_ROCKET_3
	@ Four Island / Six Island PC Blue scenes count as already over
	setvar VAR_MAP_SCENE_FOUR_ISLAND, 1
	setvar VAR_MAP_SCENE_SIX_ISLAND_POKEMON_CENTER_1F, 1
	end
