# Wally's Journey — Lore-driven wild encounters (design contract)

## Goal
Every non-legendary, non-mythical species from Generations 1–9 must be obtainable, without replacing Hoenn's original ecology. A species may be obtainable through an evolution, breeding, gift, fossil, or scripted encounter; it does **not** need a wild slot merely to complete coverage. Legendary and mythical Pokémon remain special-event encounters, never ambient grass/water/cave encounters.

## Evidence priority for each species
1. Read and compare *all available* English Pokédex flavor texts across mainline generations, noting recurring habitat, climate, behavior and ecology. Do not infer habitat from typing alone.
2. Compare actual encounter locations, rarity, encounter method, level and story progression in older and newer mainline games; note exceptional appearances separately (mass outbreaks, raids, postgame zones).
3. Match Hoenn's geography, elevation, cave depth, rainfall, water temperature, tides and terrain. Hoenn-native placements take precedence when they remain credible.
4. Balance game progression: fully evolved powerhouses should usually be evolved, not encountered early or often.
5. Record evidence, proposed map, encounter method, encounter rate, availability stage, and justification before making changes. Where sources disagree, keep a note rather than claiming definitive canon.

Useful data sources:
- PokeAPI species metadata, evolution chains, flavor_text_entries and historical version encounters: https://pokeapi.co/
- National Dex metadata, habitat, capture rate and legendary/mythical flags: https://github.com/PokeAPI/pokeapi/tree/master/data/v2/csv
- Version-specific Pokédex text and locations: https://www.serebii.net/pokedex-xy/371.shtml and https://pokemondb.net/pokedex/bagon/locations
- Current game map and encounter definitions: src/data/wild_encounters.json

## Rarity targets (DESIGN TARGETS, not verified final rates)
These percentages mean *conditional probability within the relevant encounter method*, not per walking step. Base encounter rate and rod-specific groups are separate:
- Ordinary habitat-appropriate wildlife: use existing Hoenn common slots (often 5–20%) and retain regional identity.
- Uncommon local wildlife: 2–5% conditional, preferably in one or a few suitable locations.
- Rare wildlife and unusual first-stage forms: ~1% where the chosen encounter system supports it.
- Starter Pokémon: **at most 1%** at carefully justified areas; few locations, no evolved starter wild encounters during the main story. They should feel exceptional.
- Pseudo-legendary base stages (Dratini, Larvitar, Bagon, Beldum, Gible, Deino, Goomy, Jangmo-o, Dreepy, Frigibax): **at most 1%** in late/deep/remote thematic habitat, with deliberate exceptions documented. No routine wild final evolutions.
- Other historically exceptional non-legendaries (e.g., Feebas, Spiritomb, Rotom, certain fossils): favor environmental/quest conditions over making them normal route wildlife. Rare does not necessarily mean 1% on all maps.
- If sub-1% conditional probability is essential, ordinary Emerald land tables cannot express it directly: use a tested dedicated encounter mechanic or tightly limited location/condition. Do not claim 0.1% from a 1% slot.
- Make every species line obtainable without version-only exclusivity, but do not scatter all 1,025 species through the same patches of grass.

## Source-grounded examples
- **Bagon**: original Hoenn Meteor Falls and cliff/cavern environments strongly agree with older and newer Pokédex entries; solitary and extraordinarily rare, ideally a 1% slot at a deep Meteor Falls map. Currently appears multiple times at 10%, 10%, and 5% on MAP_METEOR_FALLS_B1F_2R, which is far too common under the new rule. Also audit the current wild Salamence there: a level-39 Salamence is inconsistent with its normal evolution stage.
- **Goomy**: all-generation text repeatedly emphasizes cool, damp shade and drying danger; Route 119's rainy, shaded habitat fits well, retaining a 1% land slot, not arid Route 111.
- **Hoenn starters**: Treecko on Route 101, Mudkip on Route 103, Torchic on ash-covered Route 113 each currently occupy 1% land slots. Reevaluate habitat individually (Mudkip favors mud/wetlands; Treecko favors trees; Torchic needs defensible sheltered habitat) before expanding placements.
- **Beldum/Gible**: both currently occupy the two 1% slots in deep Meteor Falls. A full rarity rebalance must assign Bagon its own suitable 1% slot **without displacing either species from overall obtainability**. Consider other suitable late-game maps rather than concentrating every pseudo-legendary in the same room.
- **Evolution levels**: do not place middle/final forms below ordinary evolution thresholds unless there is a documented mainline precedent and deliberate design justification.

## Encounter-table mechanics that must be respected
Current field slot weights:
- Land: [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1].
- Surf and Rock Smash: [60, 30, 5, 4, 1].
- Fishing grouped by rod: Old [70, 30], Good [60, 20, 20], Super [40, 40, 15, 4, 1].
Repeat appearances sum: two 1% slots mean 2%; a 10% + 5% placement means 15%. Do not mistake encounter slots for occurrence probability. An encounter rate of 20 on land is a separate roll for entering a battle.

## Mandatory checks before merging encounter changes
1. Build a machine-readable species-by-species research ledger (sources, Pokédex ecology, historic location precedents, Hoenn placement, rarity, evolutionary availability).
2. Compare each line against all obtainable forms and evolution/breeding/gift/scripted access, not just wild slots.
3. Audit effective probabilities by **map + method + rod** and flag rare-category species above their caps.
4. Audit high-tier wild forms below normal evolution levels and obvious environmental contradictions.
5. Ensure no original Hoenn native becomes unobtainable and no forbidden legendary/mythical species enters ambient encounters.
6. Compile and test actual wild encounters in an emulator before declaring the distribution finished.

Status: design policy recorded; research, global rebalance and emulator tests remain to be done. This document does not claim that every Pokédex entry has already been reviewed.
