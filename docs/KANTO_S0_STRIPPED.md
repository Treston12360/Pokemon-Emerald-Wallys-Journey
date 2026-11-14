# S0 — FireRed's own story taken out of Kanto (log)

Owner (Treston, 2026-11-13): strip FireRed's story scripts map by map (ARIA is set after FireRed).
Two tools:
- `tools/aria_strip.py MAPDIR SCRIPT...` removes the trigger / on-frame entries that start a FireRed scene (the
  scripts stay in `scripts.inc`, unused, so they can be restored).
- `data/scripts/aria_kanto_start.inc` (`EventScript_AriaKantoStart`, run once per save by `SaveBlockPC_InitKanto`)
  sets FireRed's new-game hidden people, then hides or finishes FireRed scenes we don't play.

**Kept on purpose** (pending the road-block plan, `docs/KANTO_PROJECT_ARIA.md` §4): Pewter's Gym guide stops the
player leaving east before beating Brock (fits ch. 2); Viridian Gym door locked; Route 22 north gate badge
check; Pewter Museum fee; Pallet sign lady (her lines get rewritten in S-TALK).

| Map | Removed | Why |
|---|---|---|
| Pallet Town | Oak stops you at the north exit (2 triggers); Oak's National Dex rating scene | FireRed new-game / post-game scenes |
| Pallet, player's house 2F | First-time warp-in scene | FireRed's opening |
| Oak's lab | Starter scene (3 on-frame), leaving-with-starter and rival battle triggers (6) | FireRed's opening; ch. 1 is ours |
| Oak's lab (start state) | Starter Poké Balls, Pokédexes, Blue hidden | Player already has a team and Pokédex; Blue is placed by ch. 1 |
| Viridian City | Old man lying in the road + catching tutorial (3 triggers); old man starts "normal" (VAR 2) | FireRed's parcel errand |
| Viridian Mart | Oak's Parcel scene | FireRed errand |
| Route 22 | Early and late rival battles (6 triggers) | Blue's role is ours (owner: first meeting on Route 22 is ours, §6b) |
| Pewter City | Running Shoes aide (3 triggers) | Player already has Running Shoes |

## Step 2 — all FireRed Rockets and Blue rival scenes (owner, 2026-11-14: "Remove all of them" / "Strip all")
- `EventScript_AriaKantoStart2` hides every Rocket/Giovanni/Blue object via their FireRed hide flags
  (Mt. Moon B2F, Cerulean, Celadon + Game Corner + Celadon house 1 (its two Rockets had no flag; now
  `FLAG_HIDE_CELADON_ROCKETS`), Pokémon Tower, Rocket Hideout, Saffron, Silph Co., S.S. Anne, Route 22,
  Mt. Ember, Icefall Cave, Five/Six Island, Indigo Plateau credits Blue), and marks the Four Island and
  Six Island PC Blue scenes done.
- Coord triggers removed (tools/aria_strip.py): Cerulean rival ×3 + grunt ×2, Route 24 Rocket ×2,
  S.S. Anne rival ×3, Pokémon Tower 2F rival ×2, Silph 7F rival ×2, Silph 11F Giovanni ×2,
  Mt. Ember password ×3, Icefall Cave Lorelei/Rockets ×3, Five Island warehouse admin ×3.
- **Not touched, owner to decide:** Giovanni as Viridian Gym leader; Blue in the Champion's room;
  Super Nerd Miguel's Mt. Moon fossil trigger (not a Rocket).
- Start state is now versioned: `SaveBlockPC.kantoMapFlagsSet` = steps applied; `SaveBlockPC_InitKanto`
  runs only the newer steps on an older save. Never change a released step — add a new one.
- Tested (Linux build, owner_brawly save): Mt. Moon B2F has no Rockets; Cerulean has no Blue/Rocket.

## Step 3 — Bill, and the Rocket-era road blocks (owner, 2026-11-14)
Owner: Bill → "Strip Bill's scene"; Snorlax / Tower ghost / Saffron guards → "Remove them now";
Sevii Islands → "We will do a overhaul for sevii islands later" (Sevii untouched apart from step 2's Rockets/Blue).
- `EventScript_AriaKantoStart3`: Bill's scene counts as done (`FLAG_HELPED_BILL_IN_SEA_COTTAGE`), Bill and
  Clefairy-Bill hidden (cottage empty until Bill gets new lines), `FLAG_GOT_SS_TICKET` set (FireRed uses it to
  open Cerulean's exits and for the Vermilion dock check; the player has Birch's ticket), both Snorlax hidden,
  Tower 6F Marowak scene done.
- Triggers removed: Pokémon Tower 6F Marowak ×2; Saffron gate guards (Route 5 S, 6 N, 7 E, 8 W) ×3 each.
- Tested: Route 12 has no Snorlax, Sea Cottage empty, Route 5 gate walks through.
- Known, not changed: Pokémon Tower's ghosts still can't be identified without the Silph Scope (you can run);
  Mr. Fuji stays hidden on 7F. Raise with the owner in S-BLOCK.
