# Spawn audit (2026-10-23)

Every wild placement on Hoenn maps that was added on top of vanilla Emerald (382 entries) was checked against:
- its typing;
- all English Pokédex entries (PokeAPI flavor text, every game it has; Gen 9 text is missing from PokeAPI, so Gen 9 species were judged by hand from their Scarlet/Violet habitats);
- where it has been found in past games (PokeAPI encounter data);
- owner rules: starters and pseudo-legendary lines are a 1% chance but can be in ordinary, reachable places; historically hard-to-get species stay scarce (Spiritomb, Rotom, babies, fossils, Lapras...); no wild final forms early; wild level never below the evolution level.

Vanilla Hoenn slots were left alone (they are the region's own ecology).

Where a fitting map had no free table slot, the species became a **rare visitor** on that map (`src/data/wally_rare_visitors.h`): an exact per-encounter chance on top of the table, level taken from the rolled slot. A slot that lost an added species gets its vanilla Hoenn species back.

## Changes

| Change | Map | Method | Species | Detail | Why |
|---|---|---|---|---|---|
| rare visitor | Fiery Path | land_mons | Tepig | 1% | fire pig that roasts berries with its nose: volcanic cave |
| rare visitor | Route 111 | land_mons | Fuecoco | 1% | fire croc that basks on warm ground: desert |
| rare visitor | Route 112 | land_mons | Chimchar | 1% | Pokédex: lives on rugged mountains; Route 112 mountain trail |
| rare visitor | Route 114 | land_mons | Litten | 1% | solitary fire cat of open country, not a graveyard tower |
| rare visitor | Route 111 | land_mons | Fennekin | 1% | a fennec fox: desert |
| rare visitor | Route 117 | land_mons | Scorbunny | 1% | field rabbit that loves to run: grassy route |
| rare visitor | Route 116 | land_mons | Grookey | 1% | forest chimp with a stick: wooded route |
| rare visitor | Route 102 | land_mons | Sprigatito | 1% | grass kitten of meadows, near towns |
| rare visitor | Route 114 | water_mons | Totodile | 1% | river croc: Route 114 river |
| rare visitor | Route 120 | water_mons | Froakie | 1% | pond frog: Route 120 ponds and marsh |
| rare visitor | Route 119 | water_mons | Sobble | 1% | shy river lizard: Route 119 river |
| rare visitor | Route 117 | water_mons | Quaxly | 1% | duckling of clean ponds: Route 117 pond |
| rare visitor | Route 118 | water_mons | Squirtle | 1% | turtle of shores and waterways |
| rare visitor | Route 105 | water_mons | Popplio | 1% | sea lion of the open coast |
| rare visitor | Route 109 | water_mons | Oshawott | 1% | otter of beaches and shallows |
| rare visitor | Route 128 | water_mons | Piplup | 1% | penguin of cold open sea |
| table slot | Meteor Falls 1F 2R | land_mons | Shelgon | 1%, lv 38-38 | pseudo-legendary line stays a 1% find |
| table slot | Meteor Falls B1F 1R | land_mons | Gabite | 1%, lv 38-38 | pseudo-legendary line stays a 1% find |
| rare visitor | Meteor Falls Stevens Cave | land_mons | Metang | 1% | pseudo-legendary line stays a 1% find |
| rare visitor | Sky Pillar 1F | land_mons | Dragonair | 1% | pseudo-legendary line stays a 1% find |
| rare visitor | Victory Road B2F | land_mons | Hakamo O | 1% | pseudo-legendary line stays a 1% find |
| rare visitor | Route 104 | land_mons | Happiny | 1% | baby, egg/rare in every game |
| rare visitor | Route 113 | land_mons | Magby | 1% | baby, egg/rare |
| rare visitor | Route 116 | land_mons | Audino | 1% | only in rustling grass in Unova |
| rare visitor | Granite Cave B1F | land_mons | Dunsparce | 1% | famously rare (Dark Cave) |
| table slot | Mirage Tower 2F | land_mons | Gimmighoul Roaming | 1%, lv 24-24 | rare roaming coins |
| rare visitor | Meteor Falls B1F 2R | land_mons | Archen | 1% | fossil Pokémon |
| table slot | Route 125 | water_mons | Mareanie | 1%, lv 25-30 | uncommon, SOS-only |
| rare visitor | Route 130 | water_mons | Lapras | 1% | gift or once-a-week in every game |
| rare visitor | New Mauville Entrance | land_mons | Elekid | 1% | baby, egg/rare |
| rare visitor | Victory Road B1F | land_mons | Tyrogue | 1% | baby, gift/egg |
| rare visitor | Shoal Cave Low Tide Lower Room | land_mons | Smoochum | 1% | baby, egg/rare |
| rare visitor | Route 111 | land_mons | Orthworm | 4% | desert worm (Asado Desert) |
| rare visitor | Route 111 | land_mons | Capsakid | 4% | chili of dry, sunny land |
| table slot | Mirage Tower 1F | land_mons | Flittle | 4%, lv 23-23 | psychic bird of the desert |
| table slot | Mirage Tower 3F | land_mons | Bramblin | 4%, lv 23-23 | ghost tumbleweed blown across deserts |
| rare visitor | Jagged Pass | land_mons | Tinkatink | 1% | forges hammers from scrap metal in mountains |
| table slot | Meteor Falls 1F 1R | land_mons | Minior | 4%, lv 20-20 | meteor core that falls from the upper atmosphere |
| rare visitor | Route 123 | land_mons | Mime Jr | 1% | mimic baby of gardens (Trophy Garden); scarce |
| rare visitor | Rusturf Tunnel | land_mons | Roggenrola | 1% | cave rock |
| rare visitor | Route 110 | land_mons | Trubbish | 5% | garbage bag Pokémon near cities |
| rare visitor | Route 114 | land_mons | Litleo | 4% | lion cub of grassland |
| removed | Route 130 | land_mons | Oricorio | vanilla species restored | flower-meadow dancer; already a 2% visitor on Route 121 |
| rare visitor | Route 120 | land_mons | Flamigo | 1% | flamingo of wetlands |
| table slot | New Mauville Inside | land_mons | Toxel | 1%, lv 26-26 | electric/poison baby: the old power plant; scarce (gift/egg in SwSh) |
| rare visitor | Sky Pillar 3F | land_mons | Solosis | 1% | cell-like Pokémon said to come from space |
| rare visitor | Route 119 | water_mons | Basculin | 1% | river fish; joins the Blue-Striped form on Route 119 |
| rare visitor | Route 121 | land_mons | Zorua | 1% | illusion fox of forests (Lostlorn Forest); scarce |
| removed | Mt. Pyre 6F | land_mons | Zorua | vanilla species restored | one Zorua placement is enough (now Route 121, 1%) |
| removed | Mt. Pyre 2F | land_mons | Indeedee | vanilla species restored | not a tower ghost; Indeedee stays on Route 123 |
| removed | Route 128 | water_mons | Tatsugiri | vanilla species restored | second Tatsugiri entry; Super Rod one stays |
| removed | Route 134 | water_mons | Palafin | vanilla species restored | final form; Finizen is on Route 124 |
| removed | Route 109 | water_mons | Finizen | vanilla species restored | duplicate; Route 124 has it |
| removed | Route 130 | water_mons | Wimpod | vanilla species restored | scarce; Route 107 1% stays |
| removed | Sky Pillar 3F | land_mons | Squawkabilly | vanilla species restored | city parrot, not a sky tower; Route 110 has it |
| removed | Magma Hideout 3F 2R | land_mons | Golett | vanilla species restored | duplicate; Cave of Origin entrance has Golett |
| removed | New Mauville Inside | land_mons | Magnezone | vanilla species restored | final form at lv 26; Magnemite line lives here |
| removed | Mt. Pyre 4F | land_mons | Gimmighoul | vanilla species restored | already a rare visitor there (Chest form); table slot back to Hoenn natives |
| removed | Mt. Pyre 4F | land_mons | Mimikyu Disguised | vanilla species restored | Mimikyu is on the owner elusive list (<=1%); the 1% slot stays |
| table slot | Meteor Falls B1F 2R | land_mons | Bagon | 1%, lv 38-38 | pseudo-legendary line stays a 1% find (owner ecology rule) |

## Kept on purpose

- Tauros, Lickitung and Kangaskhan at 4-5% in the Safari Zone (Safari Zones are where they have always lived); Stufful at 10% in the postgame Safari (common in Alola).
- Sneasel / Hisuian Sneasel in Shoal Cave (common in icy caves in every game).
- Sinistea at 5% on Mt. Pyre (an uncommon but ordinary ghost); Mimikyu (1%), Polteageist (1%) stay per the owner elusive list.
- 67 Gen 8/9 placements flagged only because PokeAPI has no Scarlet/Violet Pokédex text; each was reviewed by hand against its in-game habitat and kept.

## Open

- **Bisharp in Seafloor Cavern is lv 35 but evolves at 52** (owner rule says wild level must not be below the evolution level). Needs an owner decision: lower Pawniard to Bisharp to about 35 (Legacy-style), or move Bisharp somewhere with lv 52+.

Tools: `tools/validate_wally_encounters.py` and `tools/check_wally_wild_species_preservation.py` now count rare visitors (and forms) as wild; `tools/check_wally_wild_rarity.py` and `tools/audit_wally_habitat_rarity.py` report 0 flags.
