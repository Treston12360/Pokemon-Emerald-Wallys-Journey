# Mapping notes — making good Gen 3 maps (research, 2026-11-15/21)

Owner rule (Treston, 2026-11-15): "before you just try", research how people make good Gen 3 maps. Read this before
designing any map (Celadon office, Saffron Gym for Mimi, Koga's and Sabrina's E4 rooms).

## Rules we follow
1. **Start from the official maps.** Use FireRed/Emerald tilesets and copy how Game Freak builds the same kind of
   room (stamp real pieces from real maps: `docs/kanto_mockups/tools/compose.py`), so edges, shadows and wall
   pieces stay official. Study the closest official room first (offices: Celadon Condominiums 3F = Game Freak's
   office; Silph Co.).
2. **Purpose first, then layout.** Know what happens in the room (who stands where, what the player reads, where
   the cutscene happens) before placing furniture.
3. **Proportion.** An interior shouldn't be wildly bigger than the building outside it (a small house = a modest
   room). Official interiors cheat a little; don't cheat a lot.
4. **Walkways.** Leave a clear 2–3 tile path from the door to where the player must go; furniture never blocks the
   entrance mat's path.
5. **Against the walls:** shelves, windows, paintings, doors in the top wall; desks and tables in the room with a
   floor row between them; plants in corners. The floor row under the top wall and the left column carry the wall
   shadow (official floor variants).
6. **Fill, don't clutter.** Large empty floor looks unfinished; random clutter looks noisy. Group furniture by use
   (work area, waiting area, meeting area).
7. **Points of interest.** Every map has something to look at/read; story clues are ordinary objects (a painting,
   a locked door), not highlighted.
8. **Outside: match the town.** New doors use the exact door metatiles the town already uses on the same building
   type; paths connect to the walkable network.
9. **Check in game, 2D and 3D**, before calling a map done; show the owner a picture first.

## Sources
- PokéCommunity Daily, "Make your maps stand out": https://daily.pokecommunity.com/2016/12/04/make-maps-stand
- PokéCommunity, "Any tips for a new mapper?": https://www.pokecommunity.com/threads/any-tips-for-a-new-mapper.306292
- PokéCommunity Daily, "Fangame tutorials: mapping cities": https://daily.pokecommunity.com/2017/10/31/fangame-tutorials-mapping-cities/4
- RPG Maker blog, "Mapping interiors": https://www.rpgmakerweb.com/blog/tutorial-mapping-interior
- PokéCommunity, "[TUT] How to map": https://www.pokecommunity.com/threads/tut-how-to-map.171021/
