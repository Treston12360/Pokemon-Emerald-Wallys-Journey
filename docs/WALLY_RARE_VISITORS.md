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
