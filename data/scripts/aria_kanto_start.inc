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
