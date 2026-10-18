# Pre-League experience-curve audit (Wally's Journey)

Status: **baseline extracted; cumulative EXP and accessibility simulation not yet completed.**

## Acceptance criterion

A player who defeats **every trainer accessible before Sidney** (including optional trainers, but excluding rematches/postgame and fights not yet accessible) should enter the League with six trained Pokémon averaging **1 to 3 levels below Sidney's average party level**.

Current `TRAINER_SIDNEY`: **51 / 51 / 52 / 52 / 53**, average **51.8**, ace **53**.
Thus player full-party mean target: **48.8 to 50.8** (roughly level 49–50, with borderline 51 depending on distribution). Compare party means, **not solely Sidney's ace**. This is an acceptance target, not a claim that the present EXP curve meets it.

## Verified boss levels from `src/data/trainers.party`

| Trainer | First/main party levels | Party average |
| --- | --- | ---: |
| Roxanne 1 | 13, 12, 15 | 13.33 |
| Brawly 1 | 17, 16, 19 | 17.33 |
| Wattson 1 | 24, 23, 24, 25 | 24.00 |
| Flannery 1 | 29, 28, 30, 32 | 29.75 |
| Norman 1 | 31, 30, 31, 33 | 31.25 |
| Winona 1 | 36, 37, 36, 37, 38 | 36.80 |
| Tate & Liza 1 | 44, 44, 43, 43, 45, 45 | 44.00 |
| Juan 1 | 41, 41, 43, 43, 46, 45 | 43.17 |
| Sidney | 51, 51, 52, 52, 53 | 51.80 |
| Phoebe | 52, 53, 52, 52, 54 | 52.60 |
| Glacia | 54, 53, 53, 54, 55 | 53.80 |
| Drake | 54, 55, 53, 55, 56 | 54.60 |
| Wally Sootopolis | 53, 53, 54, 55, 56, 58 | 54.83 |
| Wally Victory Road 1 | 50, 51, 52, 53 | 51.50 |
| Wally Victory Road 2 | 52, 52, 53, 53, 54, 55 | 53.17 |
| Wally Champion | 61, 62, 62, 63, 63, 65 | 62.67 |

**Important:** `TRAINER_WALLACE` currently averages 76 (74–79); `TRAINER_JUAN_1` is 41–46. Do not mistake Wallace's later party for the eighth Gym's first battle. Verify the map scripts select the intended trainer variants, including Wally's story events, before editing levels.

## Required quantitative implementation

1. Inventory **each trainer encounter actually reachable** before entering Sidney, not just all parties in the trainer data file. Deduplicate trainer IDs by route/event and exclude rematches, optional postgame encounters, and branches that are mutually exclusive.
2. Calculate trainer Pokémon EXP awards with the game's **actual EXP generation setting, held item modifiers, trainer EXP bonus, traded Pokémon bonus, affection, EXP Share/EXP All distribution, level scaling, and growth-rate tables**. Separate experience earned by the active Pokémon from distribution to bench members.
3. Run deterministic route-by-route simulations with **six Pokémon** representing at least Medium Fast, Medium Slow, Slow, and Fast growth rates; initialize at levels obtainable at that stage and include EXP at capture where relevant. Simulate all mandatory battles, all available trainer battles, and representative optional wild encounters separately.
4. Compare **all-trainers**, **most-trainers**, and **mandatory-only** playstyles. Hard gate: all-trainers six-member average arrives at 48.8–50.8 before Sidney. Document end-of-stage levels at each Gym, Victory Road entrance/exit, and Sidney.
5. Fix discrepancies primarily by adjusting **reachable trainer density/EXP availability and distribution**, without gratuitous boss-level inflation or an indiscriminate global EXP multiplier. Check exp-all on/off and late-added teammates.
6. Re-run tests after changes and confirm Wally's Sootopolis / Victory Road battles can be lost without softlocking story progression; their unusually high levels are intentional difficulty decisions that require separate validation.

Do **not** claim the target is achieved until this simulation and an in-game progression validation pass are run.

## Source of baseline

`src/data/trainers.party` on `wally-journey-development`, inspected October 17, 2026. Baseline party file blob SHA: `7e0d67db93b6303b8e0903a5c3d256f7e7b28f7d`.
