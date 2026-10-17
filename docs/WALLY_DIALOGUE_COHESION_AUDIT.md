# Wally's Journey — Dialogue cohesion audit (2026-10-17)

**Status:** Source-level story continuity and dialogue revisions committed on a stacked audit branch. This is **not** proof of a complete emulator playthrough or that every unchanged NPC line in the entire expansion has been individually reviewed. `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md` is authoritative, with later owner decisions superseding earlier drafts.

## Canonical player-facing sequence

| Progress | Wally's state and public knowledge | Dialogue/trigger verification |
| --- | --- | --- |
| Petalburg/Mauville | Shy, grateful, insecure; family worries, player helped him catch Ralts | Existing Norman tutorial and Mauville battle retained |
| Verdanturf/Lavaridge | Determined, healthier; learns independence | Updated Verdanturf geography (hills and clean air, **not sand**) |
| Fortree | Confident sliding into dismissiveness | Existing rival speech acknowledges Norman and Winona |
| Lilycove | Contemptuous about being seen as a student; Wallace asks what he's proving | Existing rival speech retained |
| Mossdeep | **Not Champion.** Has eight badges, prepares for League challenge, wants to prove Wallace wrong | Existing rival speech explicitly refers to future Elite Four |
| Sootopolis immediately before Gym 8 | Crisis has subsided, recovery continues. Wally fights the player at the Gym door with a very difficult six-Pokémon team; **either win or loss must progress without ordinary whiteout** | Sootopolis lines amended; two outcome dialogue paths statically identified; emulator behavior **not yet certified** |
| Leaving Gym 8 after Rain Badge | Steven takes player to Juan's house; **only here** is Wally's victory over former Champion Wallace broadcast. Wally won **without Mega Evolution** | Source and story bible aligned on post-badge broadcast; emulator trigger not yet certified |
| Victory Road entrance | Wally **is Champion**, fights with just four Pokémon and without Mega; two distinct player win/loss reactions, both allowing continuation | Script and text branches inspected; needs emulator verification |
| Champion's room | Wally belittles the player then loses, recognizes his insecurity and apologizes sincerely; remains fiercely competitive but respects others | Wally apology kept; May/Brendan, Birch and Wally's Hall of Fame follow-up dialogue edited for tonal/causal continuity |
| After player becomes Champion | Wally is **former** Champion; his parents must react to defeat and humility, not talk as if his title remains current | Both parents now branch first on `FLAG_SYS_GAME_CLEAR` before their broadcast-only lines |
| Later defense and rivalry | Wally continues to train and challenge, rather than disappearing after Victory Road or his defeat | Existing Champion defense and late Wally dialogue preserved |

## Specific corrections made

1. **Wally's parents, Petalburg house:** gave both existing NPC conversations a player-Champion branch. Corrected the father's premature reference to Wally's repeated title defenses. Renamed old script labels that incorrectly implied the player had already met Wally at Ever Grande when only the TV broadcast had occurred.
2. **Route 119 May dialogue:** she no longer claims to be giving an item when the updated game correctly awards Fly via Winona. She discusses Birch's migration research and points to Winona's Feather Badge/HM. Preserves `FLAG_RECEIVED_HM_FLY` for event compatibility; no extra item is given.
3. **Wally's Verdanturf speech:** removed a misleading reference to sand in this grassy, clean-air town.
4. **Wally's Sootopolis speech:** no longer declares every resident safe immediately after the crisis; recognizes recovery work.
5. **Champion room:** softened tonal whiplash after Wally's apology; revised May/Brendan's surprise and congratulations, Birch's transition into his existing Pokédex review, and Wally's final Hall of Fame instructions. No new scene, character, battle, reward or event progression.
6. **Verdanturf family:** gave Wally's existing aunt and uncle conversations post-player-Champion branches; they no longer call Wally the current Champion after his defeat.
7. **Sootopolis crisis:** revised existing Maxie, Archie, and Steven dialogue so their motives, responsibility, Primal danger and Wallace's legend connect more credibly.
8. **Drake:** removed the premature assertion that Wally has already learned humility before the player's Champion battle; his advice now foreshadows that growth.
9. **Norman and Petalburg School Kid:** fixed extra conversations that still called Wally the current Champion after the player had defeated him; corrected Norman's Surf explanation to match badge/HM permission without teaching.
10. **Rustboro Cutter's house:** reworded the active CUT advice so it no longer contradicts the no-HM-teaching feature.
11. **Story bible and Juan's house comment:** corrected obsolete pre-Gym title broadcast timing and outdated optional-battle language. Older design history remains marked as superseded.

## Region-wide dialogue source inventory

The first automated inventory reviewed **974 script files and 8,324 labeled dialogue blocks** in map/common scripts. It surfaced **24 candidate blocks mentioning both a Champion and Wally or Wallace**, and **31 candidate HM-gift wording blocks** (some are original FireRed/LeafGreen strings or unused historical labels). The inventory is a finding aid, not runtime coverage.

- Verified that Cave of Origin and Juan's Sootopolis-Gym speeches call Wallace Champion **before** the post-Badge-8 title broadcast.
- Verified that Verdanturf's main-town NPC and Petalburg's outdoor Wally's mother already had separate post-player-Champion branches.
- Corrected missing post-player-Champion branches in Wally's own house, Wanda's house, and Petalburg House 2. Norman's postgame Gym conversation was also corrected.
- Confirmed that the original extra HM-gift text in Rustboro Cutter's house, Mauville's Rock Smash Dude house and Steven's Mossdeep home is mostly retained as unused historical labels rather than active new gifts; rewrote Cutter's actively callable help dialogue.

## Consistency checks completed at source level

- First Champion speech still identifies Wally as the incumbent and Wallace as the one he defeated.
- Broadcast, Victory Road, initial Champion battle and postgame title-defense labels are distinct stages.
- Wally does not claim to be Champion at Mossdeep.
- May/Brendan dialogues remain split correctly by player gender in the existing Champion-room event.
- In Route 119 no Fly HM item is awarded by May/Brendan; Winona's Gym is the intended source.
- Both parental game-clear checks run before the TV-broadcast dialogue in their existing NPC events; the same precedence was added to Wally's aunt and uncle in Verdanturf.
- Drake's battle-closing lesson, Archie and Maxie's apologies, and the Champion-room ending now support the same story theme without assuming Wally has already changed before he loses.
- Both Sootopolis and Victory Road battles have distinct win/loss dialogue references; this is **static script inspection, not gameplay proof**.

## Outstanding certification and wider audit

- Build and test the audit PR's Emerald target, then playtest the entire story with fresh saves: both genders, each Sootopolis and Victory Road battle outcome, before/after Badge 8, Champion win and return visits to Wally's parents.
- Consult the reusable candidate inventory script `tools/wally_dialogue_inventory.py` for all Champion/Wallace/Wally and HM-gift dialogue across map scripts. Candidate results require context; obsolete but unreferenced original dialogue may be harmless.
- Visit every existing NPC across milestone saves (before and after badges 3, 5, 7, 8; after Wally's broadcast; after the player's Hall of Fame) to identify obsolete original Emerald lines, particularly ones referring to Wallace as current Champion.
- Verify offscreen timing, object positions, dialogue page display, and script flags in mGBA. A source scan cannot establish these.
- Revisit Magma/Aqua arguments and each Gym/League character in a complete emulator narrative run; broad story-bible coverage does **not** certify that every script branch was observed.

The dialogue-only character-arc requirement is preserved: Wally's emotional development uses existing dialogue locations, not new story events.
