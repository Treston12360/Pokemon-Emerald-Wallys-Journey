# Gen 1–9 wild placement ledger (2026-10-17)

Changes from the obtainability pass. Placement scripts: `tools/wally_add_missing_families.py`, `tools/wally_add_missing_families_pass2.py`; level fixes: `tools/check_wally_emerald_wild_levels.py --fix`. Each slot taken held a species that also appears on another Hoenn map, so nothing lost its only spot. Rarer families too crowded for a slot use rare visitors (`docs/WALLY_RARE_VISITORS.md`).

Static audit result: every one of the 891 Pokédex numbers has an obtainable form (wild, visitor, gift, evolution or breeding). Not yet emulator-verified.

| Species | Map | Method | Slot rate (%) | Levels | Replaced |
|---|---|---|---|---|---|
| Tentacool | Abandoned Ship Hidden Floor Corridors | fishing [8] | Super 4 | 25–30 | Tentacruel |
| Tentacool | Abandoned Ship Hidden Floor Corridors | fishing [9] | Super 1 | 20–25 | Tentacruel |
| Tentacool | Abandoned Ship Rooms B1F | fishing [8] | Super 4 | 25–30 | Tentacruel |
| Tentacool | Abandoned Ship Rooms B1F | fishing [9] | Super 1 | 20–25 | Tentacruel |
| Yamask Galar | Desert Underpass | land [8] | 4 | 38–38 | Pupitar |
| Meowth Galar | Desert Underpass | land [10] | 1 | 44–44 | Silicobra |
| Growlithe | Fiery Path | land [8] | 4 | 14–14 | Rolycoly |
| Onix | Granite Cave 1F | land [8] | 4 | 7–7 | Geodude |
| Dunsparce | Granite Cave B1F | land [8] | 4 | 10–10 | Sableye |
| Pansear | Jagged Pass | land [6] | 5 | 21–21 | Spoink |
| Sawk | Jagged Pass | land [11] | 1 | 22–22 | Sizzlipede |
| Lunatone | Meteor Falls 1F 1R | land [8] | 4 | 19–19 | Zubat |
| Gimmighoul Roaming | Mirage Tower 2F | land [9] | 4 | 23–23 | Trapinch |
| Gastly | Mt Pyre 1F | land [6] | 5 | 23–23 | Shuppet |
| Pumpkaboo Average | Mt Pyre 1F | land [7] | 5 | 22–22 | Shuppet |
| Phantump | Mt Pyre 1F | land [8] | 4 | 29–29 | Trevenant |
| Sinistea Phony | Mt Pyre 2F | land [6] | 5 | 23–23 | Shuppet |
| Mimikyu Disguised | Mt Pyre 4F | land [8] | 4 | 27–27 | Duskull |
| Misdreavus | Mt Pyre 5F | land [8] | 4 | 27–27 | Duskull |
| Murkrow | Mt Pyre Exterior | land [2] | 10 | 28–28 | Shuppet |
| Meditite | Mt Pyre Exterior | land [7] | 5 | 25–25 | Vullaby |
| Cubone | Mt Pyre Exterior | land [8] | 4 | 27–27 | Wingull |
| Voltorb | New Mauville Inside | land [10] | 1 | 26–26 | Electrode |
| Magnemite | New Mauville Inside | land [11] | 1 | 26–26 | Magneton |
| Caterpie | Petalburg Woods | land [4] | 10 | 5–5 | Wurmple |
| Weedle | Petalburg Woods | land [5] | 10 | 5–5 | Wurmple |
| Paras | Petalburg Woods | land [10] | 1 | 6–6 | Taillow |
| Pidgey | Route102 | land [2] | 10 | 4–4 | Poochyena |
| Deerling Spring | Route102 | land [6] | 5 | 3–3 | Zigzagoon |
| Rattata | Route103 | land [4] | 10 | 2–2 | Wingull |
| Squawkabilly Green | Route103 | land [6] | 5 | 3–3 | Zigzagoon |
| Spritzee | Route103 | land [7] | 5 | 4–4 | Zigzagoon |
| Wishiwashi Solo | Route104 | fishing [1] | Old 30 | 5–10 | Magikarp |
| Gossifleur | Route104 | land [6] | 5 | 4–4 | Taillow |
| Pansage | Route104 | land [8] | 4 | 4–4 | Wingull |
| Krabby | Route106 | fishing [4] | Good 20 | 10–30 | Wailmer |
| Shellos West | Route109 | water [2] | 5 | 15–25 | Wingull |
| Meowth | Route110 | land [8] | 4 | 12–12 | Wingull |
| Helioptile | Route111 | land [6] | 5 | 19–19 | Sandshrew |
| Munna | Route112 | land [6] | 5 | 16–16 | Numel |
| Ponyta | Route112 | land [7] | 5 | 16–16 | Marill |
| Spearow | Route113 | land [6] | 5 | 16–16 | Spinda |
| Throh | Route113 | land [7] | 5 | 16–16 | Slugma |
| Zangoose | Route114 | land [2] | 10 | 17–17 | Swablu |
| Mankey | Route115 | land [8] | 4 | 24–24 | Wingull |
| Ekans | Route116 | land [3] | 10 | 7–7 | Abra |
| Sentret | Route116 | land [6] | 5 | 7–7 | Taillow |
| Poliwag | Route117 | fishing [3] | Good 20 | 10–30 | Goldeen |
| Roselia | Route117 | land [4] | 10 | 13–13 | Marill |
| Budew | Route117 | land [11] | 1 | 13–13 | Seedot |
| Basculin Red Striped | Route119 | fishing [3] | Good 20 | 10–30 | Tentacool |
| Basculin White Striped | Route119 | fishing [8] | Super 4 | 35–40 | Carvanha |
| Hoppip | Route119 | land [2] | 10 | 27–27 | Zigzagoon |
| Bellsprout | Route119 | land [3] | 10 | 25–25 | Oddish |
| Venonat | Route119 | land [8] | 4 | 25–25 | Dewpider |
| Poliwag | Route120 | fishing [3] | Good 20 | 10–30 | Goldeen |
| Yanma | Route120 | land [3] | 10 | 25–25 | Oddish |
| Surskit | Route120 | land [4] | 10 | 25–25 | Marill |
| Tangela | Route120 | land [10] | 1 | 25–25 | Kecleon |
| Carnivine | Route120 | land [11] | 1 | 25–25 | Seedot |
| Cottonee | Route121 | land [7] | 5 | 28–28 | Gloom |
| Swirlix | Route121 | land [8] | 4 | 26–26 | Wingull |
| Eevee | Route121 | land [11] | 1 | 25–25 | Kecleon |
| Exeggcute | Route123 | land [7] | 5 | 28–28 | Gloom |
| Indeedee M | Route123 | land [8] | 4 | 26–26 | Wingull |
| Munchlax | Route123 | land [11] | 1 | 25–25 | Kecleon |
| Tatsugiri Curly | Route128 | fishing [8] | Super 4 | 35–40 | Wailmer |
| Lapras | Route130 | water [3] | 4 | 25–30 | Pelipper |
| Qwilfish Hisui | Route132 | fishing [8] | Super 4 | 35–40 | Wailmer |
| Diglett | Rusturf Tunnel | land [6] | 5 | 5–5 | Whismur |
| Goldeen | Safari Zone Northwest | fishing [7] | Super 15 | 30–35 | Seaking |
| Goldeen | Safari Zone Northwest | fishing [9] | Super 1 | 25–30 | Seaking |
| Lickitung | Safari Zone Northwest | land [6] | 5 | 31–31 | Rufflet |
| Doduo | Safari Zone Northwest | land [8] | 4 | 29–29 | Dodrio |
| Psyduck | Safari Zone Northwest | water [3] | 4 | 30–35 | Golduck |
| Psyduck | Safari Zone Northwest | water [4] | 1 | 25–40 | Golduck |
| Nidoran F | Safari Zone South | land [6] | 5 | 25–25 | Gloom |
| Nidoran M | Safari Zone South | land [7] | 5 | 27–27 | Wobbuffet |
| Tauros | Safari Zone South | land [8] | 4 | 25–25 | Pikachu |
| Kangaskhan | Safari Zone Southeast | land [6] | 5 | 35–35 | Hoothoot |
| Goldeen | Safari Zone Southwest | fishing [7] | Super 15 | 30–35 | Seaking |
| Goldeen | Safari Zone Southwest | fishing [9] | Super 1 | 25–30 | Seaking |
| Croagunk | Safari Zone Southwest | land [6] | 5 | 25–25 | Gloom |
| Burmy Plant | Safari Zone Southwest | land [11] | 1 | 29–29 | Teddiursa |
| Sneasel | Shoal Cave Low Tide Ice Room | land [7] | 5 | 32–32 | Snover |
| Delibird | Shoal Cave Low Tide Ice Room | land [8] | 4 | 30–30 | Golbat |
| Seel | Shoal Cave Low Tide Inner Room | water [2] | 5 | 25–30 | Spheal |
| Sneasel Hisui | Shoal Cave Low Tide Stairs Room | land [8] | 4 | 32–32 | Golbat |
| Magikarp | Sootopolis City | fishing [9] | Super 1 | 5–45 | Gyarados |
| Corsola Galar | Underwater Route126 | water [3] | 4 | 30–35 | Relicanth |
| Meditite | Victory Road 1F | land [9] | 4 | 36–36 | Whismur |
| Tyrogue | Victory Road B1F | land [8] | 4 | 42–42 | Pawniard |
