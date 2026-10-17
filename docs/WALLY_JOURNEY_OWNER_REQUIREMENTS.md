# Wally's Journey — Owner Requirements / Permanent Project Memory

**Authority:** This is the standing record of the owner's requested design for Pokémon Emerald: Wally's Journey. Read this file **before** making changes, planning work, or claiming completion. Preserve these requirements between sessions. When the owner changes a decision, update this file in the same commit. Do not silently change the design.

**Project:** A playable **GBA Pokémon Emerald ROM hack**, not a Python or PC-only game. Python is allowed for offline tooling/checks, not as a replacement for the ROM. Develop on `wally-journey-development`; verify before integrating/releasing. Build and emulator playtesting are required before calling a feature complete.

## Story, characters and world

- Emerald's core Hoenn adventure remains recognizable, but the story is rewritten/enhanced around **Wally's Journey**.
- **Norman remains the player character's father.** This is an explicit owner correction and must not be lost.
- Wally has **more rival battles** throughout the journey. His personality arc begins shy and unsure, then gradually becomes noticeably more confident, capable, and cool as the story progresses. Wally ultimately becomes **Hoenn Champion instead of Wallace**. Wallace is referred to as the **previous Hoenn Champion**.
- Wally is a prominent rival with an **extremely challenging, competitively cohesive team of six Hoenn Pokémon**. Optimize synergy, coverage, abilities, moves, held items, AI, and difficulty. Avoid copying another important character's signature ace. The precise final roster is **not confirmed here**; do not invent or freeze one without consulting the owner/record.
- May remains important as a rival/character.
- **Mega Evolution is restricted to Gym Leaders/gym trainers, the Elite Four, Champions, rivals, Steven Stone, and Battle Frontier opponents.** Ordinary route/world trainers must **not** Mega Evolve. Determine eligible gym-trainer battles deliberately; do not extend Mega Evolution to ordinary trainers.
- **Steven Stone gives the player the Mega Bracelet inside Wally's house after the player defeats Norman.** Steven acknowledges how much the player has grown, explains he found the bracelet while traveling in another region, and says he believes it can help the player reach new heights. This is the authoritative scene and timing; supersedes all earlier May/Steven and fifth-gym speculation.
- The relevant starter's Mega Stone should be available. Exact availability and Mega battle distribution still need implementation validation; do not claim they already work in the ROM.
- Gym Leader rematches should upgrade teams and grant/use a Mega Evolution where a leader did not have one previously.

## Quality-of-life mechanics

- Both **Mach Bike and Acro Bike** can be owned and used without returning to Rydel's shop to exchange them.
- **TMs are reusable/infinite**, not consumed when taught.
- **Field HM effects** still require the relevant Gym Badge/permission, but the player **does not have to teach the HM move to a party Pokémon** to use the field action.
- **HM acquisition/story dialogue overhaul:** Existing moments where other characters simply hand out HMs should use fitting normal dialogue instead where necessary; Gym Leaders will explain that unlocking an HM's field use through the badge means the player need not teach the move to any Pokémon, and the Gym Leader also awards the corresponding HM. Audit each HM, badge, map event and story dependency so the game never loses a required HM or locks progression.
- Pokémon can **relearn eligible moves conveniently**, in the spirit of Pokémon Legends: Arceus, without permanent lockout of previously learned moves. Final UI/implementation details need verification.
- An optional **game speed-up** should not pitch-shift, accelerate, or distort music; design for actual GBA emulator/hardware constraints and test rather than promising arbitrary speedups.
- **Following Pokémon:** party Pokémon can visibly follow the player in the overworld, with appropriate sprites/behavior and **correct shiny variants** for shiny Pokémon.
- **Shiny odds: 1 in 4,096** for ordinary shiny generation; audit applicable encounter, gift, breeding, and static-generation paths and document deliberate exceptions.
- **Champion title defenses:** after becoming Champion, the player should be able to defend the championship in repeatable challenges; design proper triggers, teams, and progression.

## Gen 1–9 ecology and complete Pokédex obtainability — HIGH PRIORITY

- **Every non-legendary/non-mythical Pokémon species from Generations 1–9 must ultimately be obtainable in a single-player save** through a combination of wild encounters, evolution, breeding, gifts, fossils, and appropriately implemented events/other methods. Do **not** require one ambient wild slot for every evolved species.
- Across all games, review **all available Pokédex habitat/ecology descriptions** and historical wild encounter locations, including older and newer mainline entries. Make Hoenn placements believable for climate, terrain, season/weather, tide, elevation, caves and water environment. Favor canonical Hoenn placements where appropriate. **Do not infer ecology from typing alone.**
- Preserve Hoenn's native wildlife and progression; don't indiscriminately dump every species onto routes.
- Make **starters, pseudo-legendary families, and Pokémon historically very hard to find genuinely extremely rare**: typically at most **1% conditional probability per encounter method/rod** where applicable, only in a very small number of appropriate locations. Repeated slots must be added to obtain effective probability. Check scarcity across the *whole region*, not just each map. Stronger evolved pseudo-legendaries must remain highly exceptional, if encountered wild at all.
- Treat elusive species such as Feebas, Spiritomb, Rotom and fossils through appropriate limited habitats, conditions or acquisition mechanics. **Legendary and mythical species must not appear as ordinary ambient grass/surf/cave encounters**; special encounters/events require their own explicit plan.
- A Pokémon's **wild level must never be below the level at which it evolves under this project's actual evolution rules**. Include custom Emerald Legacy-style level changes. For stone/friendship/other special evolution mechanics, manually check story progression rather than inventing a single level threshold.
- **Crucial owner clarification:** If a catchable pre-evolution can legitimately evolve into its later stage, the later stage **does not need any wild placement** if no believable location exists. All species still must be obtainable, with their actual evolution, breeding, item, form and event requirements validated. Prioritize *family obtainability + plausible ecology* over inflated counts of unique wild species.
- Preserve **Pokémon Emerald Legacy-inspired solo evolution levels** instead of relying on a **Linking Cord**. Explicitly rejected: stocking Linking Cords or requiring them for solo trade evolutions. The broader evolution design must be checked species-by-species; do not automatically force all item evolutions to become level evolutions. Known verified code examples include Machoke→Machamp 38, Graveler→Golem 38, Kadabra→Alakazam 42, Haunter→Gengar 42. Gen 5 Karrablast/Shelmet were temporarily configured to evolve at level 38; treat that as this project's provisional extrapolation, **not a verified Emerald Legacy canonical rule**.
- Single-player completion cannot depend on link trades, inaccessible version exclusives, mutually exclusive one-off evolution items, or unavailable evolution conditions. Verify item shop/pickup availability, repeatable acquisition when needed, breeding and form mechanics.

## Research and acceptance criteria

- Maintain a **species-by-species evidence ledger** listing cross-generation Pokédex observations, documented game encounter locations, habitat justification, proposed Hoenn map/method, conditional effective encounter rate, minimum wild level, evolutionary line availability and remaining proof.
- Use `src/data/wild_encounters.json` and actual species/evolution definitions as source of truth. Existing scripts under `tools/` provide **partial static checks**; they are not proof of complete obtainable Pokédex.
- Check encounter slot structure, map/method rates, rod subgroups, starters and pseudo family rarity, effective repeated-slot rate, habitat fit, underleveled evolutions, species line reachability, and no unintended legendary/mythical ambient encounters.
- The initial audit identified **261 underleveled wild slots among 1,362 checked slots**, across Hoenn plus bundled non-Hoenn maps. **22 Hoenn slots were subsequently changed to catchable earlier stages** (commit `a04f4310`), with family-aware validator updates. Those source changes are committed but require fresh full-audit execution, ROM compilation, and emulator testing; never present 261 as the current post-change count without rerunning the audit.
- Build the GBA ROM and run real emulator checks for encounters, evolutions, story, HM badge gates, TMs, bike switching, move relearning, Mega Evolution and save/load stability. Never claim the ROM or a feature is fully complete until verified.

## Status and decision hygiene

- **Requested** is not the same as **implemented**, **static-audited**, **build-passed**, or **emulator-verified**. Label status honestly in reports.
- The wild encounter ecology effort and cross-game Pokédex review are **not finished**. Do not give an invented percentage of completion.
- Prioritize the encounter/ecology/obtainability work when asked to 'continue' in this workstream, rather than diverting into unrelated trade-evolution changes.
- Related technical evidence: `docs/WALLY_WILD_ENCOUNTER_ECOLOGY.md`, `docs/wally-wild-evolution-level-audit.md`, `docs/WALLY_SOLO_EVOLUTION_ITEM_CHECKLIST.md` (legacy item checklist may contain outdated Linking Cord assumptions; this file takes precedence).
- Record future owner decisions and corrections here promptly and update conflicting docs/tests. When unresolved, state what's undecided rather than fabricate specifics.

## Current active work and resumable checklist (2026-10-16)

- **Active:** audit and correct remaining underleveled encounters without introducing implausible levels or habitats. Keep the earlier stage available; an evolved stage needs no wild placement when evolution is reachable.
- **Next:** run full static validators again, review their exceptions, and check complete evolution-chain reachability, including alternate forms, items, breeding, and event sources.
- **Then:** research Pokédex ecology and actual historical game locations for every Gen 1–9 family; maintain sourced per-species placement and rarity ledger and rebalance wild tables accordingly.
- **Later verification:** compile a playable GBA ROM, run tests and emulator playthroughs for wild encounters, evolutions, key story/QoL mechanics, and saving.
- **Not complete:** all-species obtainable proof, comprehensive historical habitat research, global rare-spawn balancing, full underlevel correction, and ROM/emulator certification.

## Explicit owner decisions added 2026-10-16

The owner specifically rejected ordinary route trainers using Mega Evolution. Eligible categories only: gym trainers and Gym Leaders, Elite Four, Champions, rivals, Steven Stone, and Battle Frontier trainers. Steven gives the Mega Bracelet at **Wally's house after Norman is defeated**, with a speech celebrating growth and describing its discovery on a journey in another region. Wally develops from shy to confident through more rival battles and **replaces Wallace as Champion**; Wallace is identified as the previous Champion. Add following Pokémon with shinies, 1/4096 shiny odds, Champion title defenses, tougher Gym Leader rematches with Megas, and Gym Leader HM explanations and awards rather than relying on former HM-giver scenes.

**Implementation status:** These are confirmed owner requirements, NOT claims that scripts, assets, logic, trainers, or emulator tests are complete.

## Recovered earlier-conversation requests requiring tracking (2026-10-16 retrospective)

These points were recovered from earlier Wally's Journey discussions and compared with this file. An **idea discussed** is not the same as a final owner-approved specification.

- **Experience sharing / EXP All — CONFIRMED:** Support ordinary EXP Share and an EXP All mode. **EXP All must redistribute/scale experience rather than grant substantially more total progression.** A player who uses normal EXP Share should not end up underleveled relative to an equivalent player using EXP All, assuming comparable battles/participation. Define precisely how participation, party size, switching, fainted Pokémon, level curves and bonuses interact; benchmark matched playthroughs and tune around comparable overall team progression. Do not promise all six individual Pokémon always have identical levels across modes, or artificially inflate total EXP. If the player can select EXP All, document the mode control and verify it works.
- **Z-Moves — CONFIRMED OPTIONAL FEATURE:** Z-Moves must have an explicit **On/Off toggle in the in-game Options menu** so players who dislike them can play without them while players who enjoy them can enable them. Default setting and whether the toggle also disables opposing trainers' Z-Moves must be designed and tested consistently; do not silently make Z-Moves mandatory. Unlock timing, Z-Crystal acquisition and which opposing trainers use them remain unresolved. The setting must persist through save/load.
- **Feebas:** Prior discussions identified **special Feebas encounter/evolution changes**. Current Route 119 Good Rod hook is an existing implementation detail; preserve it until the precise owner-approved rarity/access mechanics are reconciled with original conversation decisions. Don't assume the 10% Good Rod conditional mechanic fulfills the exceptionally elusive target.
- **Wally rival frequency:** Earlier planning mentioned **12 or more Wally battles** and a developing rival relationship/character memory. Treat 12+ as a prior planning target, **not a finalized exact battle count**; confirm story pacing and record all final battle triggers.
- **Starter Mega Stone specificity:** The post-Norman Steven sequence should grant or enable access to **Sceptilite, Blazikenite, or Swampertite as appropriate for the player's chosen starter**. A prior discussion proposed Steven granting the matching stone in the same scene. Align the final script with the owner's confirmed starter-stone requirement, not a generic stone.
- **Wally battle-team proposal (NOT FINALIZED):** A previous assistant brainstorm listed Armaldo, Mawile, Breloom, Flygon, Crawdaunt and Mega Gardevoir for Wally. This was a proposal, not confirmed owner approval. The binding criteria remain a very difficult, statistically synergistic team of six Hoenn Pokémon without copying other important trainers' aces.
- **Emulator testing:** Earlier development explicitly targeted testing in **mGBA** on a GBA-compatible ROM; a PC/Python port is not a substitute.

The above historical details must be checked against newer explicit owner instructions before implementation; newer decisions always supersede older brainstorms.

## Owner clarification: optional Z-Moves and EXP parity (2026-10-16)

- Z-Moves are requested as a game feature, **toggleable in the Options menu**. This must work for different players' preferences; it is not merely a debug switch.
- EXP All should spread/scale the experience awarded rather than create a progression advantage over normal EXP Share. **Matched game progression should result in comparable overall team strength/levels**. Compare total earned team EXP, level distribution and party-size effects with repeatable battle fixtures and playthrough checkpoints.
- Do not confuse equivalent overall progression with identical level for every party slot at every moment; avoid making EXP All users inherently stronger or penalizing ordinary EXP Share players.
- Both are **confirmed requirements, not implemented/tested features** at the time of this update.

## Battle systems, trainer AI, and implementation reuse — confirmed owner requirements (2026-10-16)

- **Physical/Special split:** Use the modern **per-move** Physical / Special / Status distinction rather than determining damage category solely from move type as in original Pokémon Emerald. Audit current engine configuration and move metadata first; if already supported, verify it rather than reimplementing.
- **Upgraded trainer battle AI:** Trainers should make noticeably smarter choices about moves, switching, type matchups, abilities, team synergy and battle strategy where appropriate. Difficulty should be **challenging but fair, never impossibly hard**. Scale intelligence and teams appropriately to trainer rank, story progression and available player resources. Ordinary trainers should not be tuned like competitive Champions, and still **must not Mega Evolve**.
- **Prefer existing compatible mods/implementations when sensible:** Before building major features from scratch, investigate existing Pokémon Emerald / pokeemerald-expansion / compatible GBA ROM-hack implementations and patches. Compare compatibility, maintenance, code quality, licensing, integration cost, behavior and testing effort. Reuse/adapt proven code when it saves effort and preserves the desired behavior; implement internally when that is simpler, safer or more maintainable. Do not blindly copy incompatible ROM patches or claim another project's feature has been integrated without testing.
- **Status:** These are confirmed requirements. Whether the current engine already supplies the physical/special split or sufficiently advanced trainer AI must be audited before calling either complete.
