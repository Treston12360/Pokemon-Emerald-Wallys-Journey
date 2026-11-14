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
