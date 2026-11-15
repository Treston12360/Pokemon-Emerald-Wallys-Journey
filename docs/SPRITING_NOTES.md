# Spriting notes — how to make Gen 3 sprites look official (research, 2026-11-15)

Owner rule (Treston, 2026-11-15): "Make sure that you are doing really, really good research on how sprites are
made and how to make them look good and how other people make Gen 3 sprites look amazing." Read this before
making or editing any overworld or battle sprite. ARIA rule (KANTO_PROJECT_ARIA §7): only FireRed/Emerald art —
so new characters are **frankensprites/recolours of official Gen 3 sprites**, not drawn from scratch.

## What worked here (and what didn't)
- Varnell: hand-drawn/auto-relit versions (v4, v5, 2x) were rejected ("looks worse", "cursed"). Built from official
  parts (Steven hair + Scientist face/glasses/coat) with hand-picked ramps → approved (v7).
- Janine: official FRLG Cooltrainer F recoloured + small additions (scarf tail, sash, pants) → approved (v2).
- Lesson: **start from the official sprite that is closest in shape/pose**, change colours and only a few pixels.

## Technique checklist
1. **Pick the base by silhouette**, not colour: hair shape, body type, pose. Colours are cheap to change, shapes aren't.
2. **Palette:** 16 entries, index 0 transparent → 15 colours for the whole sprite (Voliol, Gen III palettes).
   Official sprites share colours between materials (e.g. skin shadow = hair dark). When recolouring one material,
   check every index it uses — shared indices need a per-pixel mask (by position), not a palette swap.
3. **Ramps:** 3–4 shades per material (light, base, shadow, outline), **hue-shifted**: highlights warmer/yellower,
   shadows cooler/bluer-purpler, not just darker (PokéCommunity overworld tutorial; Dragonfly Cave guide).
4. **Light comes from the top (top-left on battle pictures)** — shade the opposite side, highlight the top of hair.
5. **Outlines:** black around clothing/hair mass in Gen 3 overworlds, **brown (123,65,65)** around skin.
   Selective/coloured outlines: an outline must be darker than both the object and the background and always add
   contrast (Lospec, "Pixel art outlines part 2"). Internal lines one shade darker than the area.
6. **Anti-aliasing:** by hand, sparingly, inside the sprite; no banding, no pillow shading, no noise on tiny sprites
   (Pixel Parmesan, "Anti-aliasing fundamentals").
7. **Overworld sprites need little detail** (PokéCommunity "any tips for overworld spriting"): minimal face,
   2-pixel eyes; glasses = pale lenses in a dark frame (like the Scientist).
8. **Seams:** where two parts meet, redraw the joining pixels and re-check the outline; mirror the sprite to spot
   lopsided parts.
9. **Check at 1× and in game**, 2D and 3D, next to official NPCs. A sprite that looks fine zoomed can be mush at 1×.
10. Keep the old version; show the owner side by side with the reference before putting anything in game.

## Sources
- PokéCommunity GBA/DS trainer overworld tutorial: https://www.pokecommunity.com/goto/post?id=5265003
- PokéCommunity "Any tips for overworld spriting": https://www.pokecommunity.com/threads/any-tips-for-overworld-spriting.355951
- Dragonfly Cave spriting guide: https://www.dragonflycave.com/spriting-guide/
- Reborn complete tutorial to spriting: https://rebornevo.com/forums/topic/16694-the-complete-tutorial-to-spriting
- Smogon "Spriting Incognito": https://www.smogon.com/smog/issue9/spriting
- Tun's trainer sprite tutorial (hair/clothes shading order): https://forums.pokecharms.com/threads/tuns-trainer-sprite-tut-c.976/
- Voliol, Pokémon palettes in Gen III: https://voliol.neocities.org/articles/genIIIpalettes
- Lospec, outlines with colour: https://lospec.com/articles/pixel-art-outlines-part-2-using-color
- Pixel Parmesan, anti-aliasing: https://pixelparmesan.com/anti-aliasing-fundamentals-for-pixel-artists
