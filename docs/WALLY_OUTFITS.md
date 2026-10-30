# Wally's Journey: player outfits

The **Outfit Case** key item (in the Bag from the start; old saves get it on Continue) opens a
menu any time on the overworld, and can be registered to Select. Your own sprite on the map is
the live preview.

- **OUTFIT**: Brendan, May, Red, Leaf, Ethan, Lyra, Lucas, Dawn, Hilbert, Hilda, Nate, Rosa,
  Calem, Serena, Elio, Selene, Victor, Gloria, Florian, Juliana.
- **SKIN**: default or six tones. **COLOR 1 / COLOR 2**: default or 23 colours (what each
  recolours depends on the outfit, e.g. Hilbert's cap and jacket).
- A keeps it, B puts back what you had, Start picks random colours.

The outfit changes your overworld sprite (walking, running, both bikes, surfing, fishing, field
moves, watering, decorating), your battle back sprite and your trainer card. Diving stays the
usual dark silhouette. Your gender (and the story's he/she text) doesn't change.

Nate, Rosa, Elio, Selene, Victor, Gloria, Florian and Juliana have no battle back sprite in
Emerald Rogue, so in battle they use Brendan's (male) or May's (female) back sprite.

## Credits

Protagonist sprites, palettes and colour layers: **Emerald Rogue** by **Pokabbie** and the
artists credited there (https://github.com/Pokabbie/pokeemerald-rogue). Thank you!

For protagonists Rogue doesn't draw every Emerald action for, the bike / surf / fishing / field
move / watering / decorating frames are built by `tools/wj_outfits/gen.py`: Brendan's or May's
frame with the outfit's head (from its walking frame facing the same way) and its colours.
`tools/wj_outfits/emit.py` writes the PNGs and the C tables
(`src/data/object_events/wj_outfit_gfx.h`, `src/data/graphics/wj_outfit_trainers.h`,
`src/data/wj_outfits.h`, `include/constants/wj_outfits.h`). Both expect a checkout of
pokeemerald-rogue at the path in `gen.py` (`R`).
