# Wally's Journey: solo evolution item accessibility

Source review on the `wally-journey-development` branch confirms **25** `EVO_TRADE` targets in Gen 1-9 species family definitions, each with an `EVO_ITEM` alternative. The static check is in `tools/check_wally_solo_trade_evolutions.py`.

**Important:** An evolution *definition* does not prove an item is obtainable. Every row below requires a reachable location, shop, pickup, quest, or other repeatable in-game method, plus emulator verification. None is certified accessible by this audit.

| Required item | Access verified? | Next verification |
|---|---|---|
| Linking Cord | No | Check shops, items on maps, and acquisition before National Dex completion |
| King's Rock | No | Confirm NPC/gift/shop/pickup availability |
| Metal Coat | No | Confirm non-trade obtainability and availability for both uses |
| Protector | No | Confirm reachable source |
| Dragon Scale | No | Confirm reachable source |
| Electirizer | No | Confirm reachable source |
| Magmarizer | No | Confirm reachable source |
| Up-Grade | No | Confirm reachable source |
| Dubious Disc | No | Confirm reachable source |
| Reaper Cloth | No | Confirm reachable source |
| Deep Sea Tooth | No | Confirm obtainable without mutually exclusive decision blocking Deep Sea Scale |
| Deep Sea Scale | No | Confirm obtainable without mutually exclusive decision blocking Deep Sea Tooth |
| Sachet | No | Confirm reachable source |
| Whipped Dream | No | Confirm reachable source |

## Acceptance criteria

- Every item is obtainable on a single save and is not locked by a mutually exclusive choice.
- Repeatable acquisition for items consumed by evolution, so a player can evolve multiple Pokémon without resetting.
- One Pokémon of each trade-evolution family tested in emulator.
- Confirm evolution animation, save persistence, Dex registration, and no trade/link requirement.
- Verify branch build first; this file is tracking evidence, not a claim that the game is complete.
