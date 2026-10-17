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

An evolved species does **not** need its own wild encounter if the player can legitimately obtain it by evolving a catchable earlier member of its family (or through breeding, gifts, fossils, or other supported mechanics). Avoid forcing unsuitable evolved Pokémon into habitats merely to preserve a wild species count.

## Correction invariants

1. All ordinary level-based evolved wild encounters must have minimum wild level at least their current project-defined evolution threshold.
2. When a too-early evolved species is replaced with its pre-evolution, a separate wild evolved encounter is OPTIONAL. Verify that the family remains catchable and the evolved form is attainable with the project's actual evolution rules.
3. Do not silently inflate early-game areas to levels 50+ or make pseudo-legendary final forms common simply to pass a validator.
4. Preserve the original Hoenn encounters unless the redesign is explicitly justified.
5. Validate starter/pseudo rarity, map slot counts, and full species obtainability by catching, evolving, breeding, gifts, fossils, or supported special encounters. Do NOT require every evolutionary stage to appear wild.
6. Evaluate stone, friendship, time, weather, and other special evolution methods separately; a numeric level minimum cannot be inferred for them.
7. Review bundled Kanto/other-region encounter data separately from Hoenn-specific redesign decisions.

Current status: findings recorded; the 261 underleveled slots are NOT corrected by this document. Full obtainability must be verified against real evolution/item/event access, not assumed from a family member being present.
