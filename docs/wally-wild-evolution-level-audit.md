# Wild evolution-level audit: correction policy

The all-generation scanner in `tools/check_wally_all_wild_evolution_levels.py` found 261 underleveled slots in 1,362 level-evolution slots examined on 2026-10-16. This includes bundled non-Hoenn encounter tables. The scanner covers only exclusively numeric-level evolution targets and is not a gameplay certification.

## Confirmed Hoenn examples requiring redesign

| Map | Species | Wild level | Evolution level |
|---|---|---|---|
| Mt. Pyre 6F | Zoroark | 22 | 30 |
| Mt. Pyre Summit | Mandibuzz | 24 | 54 |
| Magma Hideout 2F 2R | Volcarona | 33 | 59 |
| Sky Pillar 1F | Noivern | 38 | 48 |
| Route 122 | Jellicent | 25-30 | 40 |
| Shoal Cave Ice Room | Abomasnow | 32 | 40 |

Each listed evolved species currently has **only one** ordinary wild placement, so replacing it outright would drop that species from the wild roster and fail the existing evolved-species preservation design goal.

## Correction invariants

1. All ordinary level-based evolved wild encounters must have minimum wild level at least their current project-defined evolution threshold.
2. If a too-early evolved species is replaced with its pre-evolution, add a separate habitat-plausible, sufficiently leveled wild encounter for the evolved species before calling the change complete.
3. Do not silently inflate early-game areas to levels 50+ or make pseudo-legendary final forms common simply to pass a validator.
4. Preserve the original Hoenn encounters unless the redesign is explicitly justified.
5. Validate starter/pseudo rarity, map slot counts, catchability by evolution, and species preservation after edits.
6. Evaluate stone, friendship, time, weather, and other special evolution methods separately; a numeric level minimum cannot be inferred for them.
7. Review bundled Kanto/other-region encounter data separately from Hoenn-specific redesign decisions.

Current status: findings recorded; the 261 underleveled slots are NOT corrected by this document.
