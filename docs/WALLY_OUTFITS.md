# Wally's Journey: player outfits

The **Outfit Case** key item (in the Bag from the start; old saves get it on Continue) opens a
menu any time on the overworld, and can be registered to Select. Your own sprite on the map is
the live preview.

- **OUTFIT**: Brendan, May, Red, Leaf, Hilbert.
- **SKIN**: default or six tones. **COLOR 1 / COLOR 2**: default or 23 colours (what each
  recolours depends on the outfit, e.g. Hilbert's cap and jacket).
- A keeps it, B puts back what you had, Start picks random colours.

The outfit changes your overworld sprite (walking, running, both bikes, surfing, fishing, field
moves, watering, decorating), your battle back sprite and your trainer card. Diving stays the
usual dark silhouette. Your gender (and the story's he/she text) doesn't change.

Only outfits with a full set of sprites for every action are included (owner rule: no
patchwork). Emerald Rogue has 15 more protagonists, but only walking and riding art for them.

## Where the sprites come from

- Brendan, May: the game's own sheets.
- Red, Leaf: Emerald Rogue's sheets for everything except watering and decorating; those two
  are their own standing art holding the Wailmer Pail / the decoration (owner-approved).
- Hilbert: Emerald Rogue's walking, riding, trainer card and battle back art. The rest was made
  for Wally's Journey and approved sheet by sheet by the owner:
  - running: his walking frames leaned one pixel forward;
  - surfing: his seated (riding) art, and his standing frame for hopping on/off;
  - Mach/Acro Bike: his seated art on the bike, with his legs pedalling in his trousers/shoes;
  - fishing, field move, watering, decorating: his standing art holding the rod / Poke Ball /
    Wailmer Pail / decoration.
  The approved PNGs are kept in `tools/wj_outfits/approved/`; the scripts that made them are
  next to them (`hilbert.py`, `hsurf.py`, `hbike.py`, `hprop.py`); `emit.py` copies them in
  and writes the C tables.

## Credits

Protagonist sprites, palettes and colour layers: **Emerald Rogue** by **Pokabbie** and the
artists credited there (https://github.com/Pokabbie/pokeemerald-rogue). Thank you!
