# Rare visitors (sub-1% encounter mechanic)

Normal tables can't go below a 1% slot. A rare visitor gets its own exact chance per encounter of that type on that map, rolled on top of the normal table (`src/data/wally_rare_visitors.h`, roll in `WallyJourney_RollRareVisitor`, `src/wild_encounter.c`). Level comes from the slot rolled. Repel and Keen Eye checks still apply.

| Species | Map | Type | Chance per encounter |
|---|---|---|---|
| Flabebe Red | Route117 | Land | 2% |
| Comfey | Route117 | Land | 0.5% |
| Farfetchd Galar | Route117 | Land | 0.5% |
| Milcery | Route121 | Land | 0.5% |
| Poltchageist Counterfeit | Route123 | Land | 1% |
| Wooper Paldea | Route114 | Land | 2% |
| Cleffa | Meteor Falls 1F 2R | Land | 0.5% |
| Drowzee | Route114 | Land | 3% |
| Eiscue Ice | Shoal Cave Low Tide Ice Room | Land | 1% |
| Foongus | Route120 | Land | 3% |
| Panpour | Route120 | Land | 2% |
| Klefki | New Mauville Inside | Land | 2% |
| Porygon | New Mauville Inside | Land | 0.5% |
| Morelull | Petalburg Woods | Land | 2% |
| Morpeko Full Belly | Route110 | Land | 2% |
| Zigzagoon Galar | Route113 | Land | 0.5% |
| Oricorio Baile | Route121 | Land | 2% |
| Scyther | Safari Zone Northeast | Land | 2% |

Status: code-in and builds; real odds need an emulator sample to confirm.

## Alternate forms (added 2026-10-17)

55 more visitor rows, all listed in `src/data/wally_rare_visitors.h` with their exact chance:
- Alolan Rattata, Sandshrew, Vulpix, Diglett, Meowth, Geodude, Grimer; Galarian Ponyta, Slowpoke, Darumaka, Stunfisk; Hisuian Growlithe, Voltorb, Zorua; Paldean Tauros (Combat, Blaze, Aqua) in the Safari Zone.
- Blue-Striped Basculin (Route 119 water), East Sea Shellos (Route 124 water), Sandy/Trash Burmy, four more Flabébé colors, Pumpkaboo Small/Large/Super, Squawkabilly Blue/Yellow/White, Tatsugiri Droopy/Stretchy, Antique Sinistea and Artisan Poltchageist (0.2% each), female Indeedee, Chest-form Gimmighoul.
- All 20 Vivillon patterns, 0.5% each, spread across Hoenn by climate.

Other routes to forms:
- Deerling/Sawsbuck wear the coat of the real-world season (`WallyJourney_SeasonalForm` in `src/wild_encounter.c`).
- Lilycove Department Store 2F sells the Rotom Catalog and the four Oricorio nectars.
- Postgame Fan Club collector in Slateport gives one per talk: 8 cap Pikachu, 6 costume Pikachu, Spiky-eared Pichu, Battle Bond Greninja, Own Tempo Rockruff, Eternal Flower Floette (`VAR_WALLY_FAN_CLUB_GIFTS`).
- In-battle forms (Aegislash Blade, Castform weather forms, Zen Darmanitan, Busted Mimikyu, Hangry Morpeko, Hero Palafin, School Wishiwashi, etc.) come from owning the base form and are excluded from the audit.

