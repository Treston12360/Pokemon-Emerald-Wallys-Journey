# Project ARIA — complete narrative scene pass (DRAFT, not approved)

**Prepared November 16, 2026. Status: NEW PROPOSALS / OWNER REVIEW.** Source of truth for decisions is `docs/KANTO_PROJECT_ARIA.md`; see `docs/KANTO_NARRATIVE_AUDIT.md` for locked causality and `docs/KANTO_CONTINUITY_SOLUTIONS_DRAFT.md` for proposed mechanisms. This document is **not playable game script**. Varnell's opening portal line in Chapter 13 is owner-approved (2026-11-17). Other proposed lines still await review. Preserve old, approved lines that do not conflict. **Treston's ARIA story only — no spoilers to Brennan / Polish.**

## Narrative rules, before writing a single new script

- **Protagonist:** the exact Hoenn player character; NOT Wally. Wally is back in Hoenn and calls occasionally.
- **Blue:** former Champion, nearly Red's equal; experienced researcher; skeptical of this outsider at first, not a quippy bully. He works independently at first, then shares evidence, then respects the player. He **never suspects Varnell before Chapter 12**. He knew Varnell lost a daughter but never knew her name. The betrayal is personal.
- **Varnell:** kind, credible, genuinely helpful in public; once aided Blue after Red. He knows his experiments could kill countless people and willingly accepts that risk to retrieve Aria. Do not conflate authentic affection and ethical innocence. He thinks Rocket caused his child's death; he never knows his own reverse jump is the immediate cause.
- **Rocket:** disbanded in present Kanto. The Tower raid, stolen Clefairy, stolen research, and old Game Corner records refer to *historical* Rocket. No new Rocket syndicate.
- **Aria:** living child in the past window; Clefairy's spirit is encountered in the present. Distinct states, not a ghost summoned to the present.
- **Three reveals:** daughter/name may be deduced before Ch.12; actual purpose of ARIA comes in Ch.12; fatal fixed time loop after Varnell's jump in Ch.13. Optional hints reward replay; all mandatory revelations must work if optional hints were skipped.
- **Unbreakable finale:** stable receiving window, Aria refuses without captive Clefairy, Varnell fears her recorded death is seconds away, Blue warns of reverse direction, Varnell jumps **first**, *then* destination starts failing, *then* Clefairy escapes Rocket and runs to Aria in falling rubble, *then* three die. No roof shaking or escape beforehand. Corpse not explicitly identified; nobody narrates what happened.

### Story status by segment

| Segment | Current reference status | This document |
| --- | --- | --- |
| Ch.0, 0b, 1, 2, 3b | Already approved / in game (see `KANTO_SCRIPT.md`) | Do not replace; only continuity notes. |
| Ch.3, Kiyoshi rescue | Old approved **but now contradicted** forced closure is IN GAME | Below is a **new draft replacing only scenes 3.4, 3.5, 3.6** after owner approval. |
| Ch.4, Lavender locals | Old approved / in-game resident incorrectly sounds certain Rocket caused death | Below is a **draft NPC replacement**; do not change other Chapter 4 scenes automatically. |
| Ch.5 | Draft/approved direction exists in `KANTO_SCRIPT.md` | Keep existing text; add optional late revisit clue; no unapproved rewriting. |
| Ch.6–14 | Outline locked, no final approved dialogue | Story beats and **new dialogue drafts** below. |

## Historical raid: private scene-blocking proposal (not player-facing)

**DRAFT staging, pending owner choice:** Young Varnell and Aria are separated during the raid. A Rocket member forces young Varnell out of the tower; he cannot safely reenter. Another Rocket member still holds Clefairy captive in an upper corridor; Aria searches within the tower. Do **not** give Clefairy an escape before the finale.

**Past time-line camera logic:** The portal view lets the player see Aria in the upper Tower passage and the Rocket captor nearby/out of her reach. The portal is a *receiving* window, opening over successive real-time seconds in that past moment; the characters can hear each other. Its normal operation creates no shaking. Varnell knows from historical accounts the collapse supposedly happened shortly afterward, so she appears to be running out of time. His conviction is based on false attribution to Rocket.

After he jumps, the wrong-direction mass/energy overload at the **past endpoint** begins shaking the structure. The Rocket member loses hold of Clefairy in the panic (exact escape mechanic not locked); Clefairy runs to Aria as the roof falls; future Varnell is near them; all three are caught. A flash/cut to present ends the view. **Do not add a fourth dead Rocket without separate approval.** Do not show Varnell's badly damaged body on-screen; the earlier anonymous casualty does that work through inference.

## Chapter 3 — Kiyoshi: revised scenes 3.4–3.6 — **APPROVED 2026-11-20 (Treston) · IN GAME** (data/scripts/aria_ch3.inc)

**Reason for change:** The currently implemented version has Varnell command the player to close a tear and knowingly return Kiyoshi to a fire; owner later specified Varnell tries to save him and the unanchored time connection expires by itself. Preserve Kiyoshi's fragility, his connection to the Brass Tower fire, his weak trainer fight, and his owner's original last cry.

**3.4 After battle, on map (draft):**

KIYOSHI: I can still feel the heat. Even here. As though I'm standing in two places.

BILL: The signal is fading. There's something pulling him back through.

(Scanner beeps. PokéNav rings.)

VARNELL: {PLAYER}, I'm receiving your readings. That man—he's not coming through from somewhere else in Kanto.

VARNELL: The years don't match. Not remotely.

BILL: Can you stop it?

VARNELL: I'm trying. The scanner may be able to hold the opening steady. {PLAYER}, point it toward the tear. Don't close it.

(Scanner turns toward the tear. The pulses briefly slow.)

KIYOSHI: It's quieter. ...Please. Don't let it take me back.

VARNELL: Hold steady. I'm adjusting the field from here.

(Brief silence, then pulses speed up; no player choice to seal.)

BILL: It won't stay open!

VARNELL: I know. I know. Give me another moment.

KIYOSHI: No... the heat... I can feel it again.

KIYOSHI: It burns! IT BURNS! AHHHHH!.....

(The connection expires by itself; Kiyoshi vanishes. No 'Rift sealed' success text.)

**3.5 Bill afterward (draft):**

BILL: We didn't send him back.

BILL: It just... stopped holding him here.

BILL: I thought if I understood where he came from, I could help him stay. I didn't know there was a difference.

**3.6 Varnell follow-up (draft):**

VARNELL: {PLAYER}. I'm sorry. I thought we could hold the connection longer.

VARNELL: I keep thinking about what he said. A person, taken out of his own time. Whole. Alive.

VARNELL: And yet we couldn't keep him here for even a few minutes.

VARNELL: Bill's right to be upset. So am I. ...I'll see what can be learned from the readings.

VARNELL: For now, go see Misty. You've done more than enough today.

**Second-playthrough truth:** Varnell genuinely tried to rescue Kiyoshi AND learned that uncontrolled time travel exists; this is the first reason he realizes he must control **when**, hence Dialga. No on-screen Dialga name.

## Chapter 4 — Lavender ordinary resident — **APPROVED 2026-11-20 (Treston) · IN GAME** (Aria_Ch4_Text_OldMan)

**At an ordinary street NPC, not a highlighted mandatory cutscene.** No music sting or quest marker.

OLD RESIDENT: I remember when Team Rocket came through the tower. Terrible day.

OLD RESIDENT: There was a little girl looking for her Clefairy. Someone had taken it, so they said.

OLD RESIDENT: They say she stopped all of a sudden, like something caught her attention. Then part of the ceiling came down.

OLD RESIDENT: Nobody ever found out exactly what happened. Most figured Rocket must've been doing something up there.

OLD RESIDENT: They found a man in the rubble too. His body was so badly damaged nobody could recognize him. Some thought he was one of the Rockets.

OLD RESIDENT: Awful thing, what they did to this town.

**Note:** Resident's beliefs about Rocket are credible rumors, **not objective history**. Avoid identifying the girl's father, connecting the unidentified man to her, describing him as reaching for her, or mentioning a portal. Preserve the previously approved tower ghost and Varnell's red-herring interpretation in Celadon. The clue is completely optional.

## Chapter 5 — Celadon: preserve current report-back and give a natural later revisit

**Keep the already-drafted Ch.5 conversation:** Blue thinks Clefairy is saying the name of the ARIA machine; Varnell hesitates exactly once, then accepts that explanation. Don't add a guilt close-up. This is a valuable false solution.

**Proposed Ch.7 return:** Varnell's staff request the player to copy a diagnostic export for the next anomaly investigation from an ordinary computer in the public Celadon office. The computer *must* supply useful quest data whether the player checks anything else or not. A separate **optional**, uncaptioned photo of a young girl with Clefairy sits near a staff storage space the player can access on this return, not Blue's normal shared desk. Examine: "An old photograph. A little girl is holding a Clefairy." No reaction animation, no name, no marker. This photo is not the only way the plot makes sense.

## Chapter 6 — Saffron: the believable scientific crisis (DRAFT)

**What happens:** League and Silph staff isolate a malfunctioning teleport facility; Mimi's Normal Gym remains available only when the quarantine clears. Varnell's public stabilization hardware and Silph's real teleport experiments overlap. Secretly, a hidden Hoopa chamber uses Silph's endpoint measurements to draw and contain Palkia; that strain causes the building's warped teleport maze. The player fixes the public-facing symptoms without seeing Palkia. Blue is sincerely helping, not suspicious.

**Scene, with Silph scientist:**

SILPH ENGINEER: The doors still work. The problem is where they send you.

SILPH ENGINEER: We can send an object across this building a hundred times and have it arrive a fraction off every time. That's never happened before.

BLUE: Can you isolate the machines on this floor?

SILPH ENGINEER: We did. Whatever's shifting the destinations isn't originating here.

BLUE: Then we narrow it down. {PLAYER}, take the east corridor. I'll check the control room.

**Mid-crisis call, Varnell:**

VARNELL: Silph's receiving arrays are reading the same instability as the tears. Their people need help, {PLAYER}.

VARNELL: I can get them a stable coordinate reference. It's experimental, but it may be enough.

(Scanner is used; ordinary public containment improves. Secret experiment data completes the spatial anchor.)

**After quarantine (short):**

BLUE: They made it sound like the entire building was going to fold in on itself.

BLUE: ...You handled your side well.

**Hidden Palkia signal for hindsight:** a terminal reports "receiver drift eliminated / secondary load unresolved"; no legendary name and no explicit secret-room map marker.

## Chapter 7 — Fuchsia: Blue begins to respect the player (DRAFT)

**What happens:** Janine's Ghost Gym showcases a different discipline from her father's Poison specialization; any Pokémon from other regions need a separate owner-approved team explanation. Safari-related anomalies give a legitimate reason for Blue and the player to compare notes. Blue trusts the player's observations, *not* because he has discovered Varnell's motive.

**Brief encounter:**

BLUE: I checked what you found in Saffron.

BLUE: Your measurements were right. Two readings that should've matched didn't. I missed it the first time.

BLUE: There's another report out by the Safari Zone. I was going to look at it alone.

BLUE: ...You know what you're doing. Come along.

**Power Plant handoff:** An ordinary staff report describes unstable power draw across the region, giving a reason to revisit the Plant *from Fuchsia by Fly* before the Seafoam segment. The public need to restart a grid relay is genuine; secretly, its power charges Varnell's temporal containment for Dialga. Do not imply the Power Plant caused Kiyoshi's earlier incident; Dialga is captured **later**.

**Celadon revisit:** user gets the mandatory export from the terminal, optional photo available as above.

## Chapter 8 — Seafoam / Power Plant: the time-anchor consequence (DRAFT)

**Public events:** With the player's help a regional power relay stabilizes; brief wrong-time effects are reported (unusual clock readings, diaries with shifting dates). Seafoam and Blaine's temporary Gym are affected by anomaly currents.

POWER PLANT ENGINEER: The machines are drawing too much, but the grid isn't delivering it anywhere we can find.

POWER PLANT ENGINEER: If the relay goes, half these stations will go with it.

VARNELL (phone): The repairs should stop the fluctuations. I'll take responsibility for the readings afterward.

**Private cause:** The prepared hidden containment bank uses that power to pull/weaken/capture Dialga. No text says it yet. A follow-up public anomaly looks like interrupted timing, not a sign that Kiyoshi was deliberately returned to death.

**Proposed decisive experiment after both captures (late Ch.8 or Ch.9):** With coordinates and time pinned to the old tower, the machine maintains a stable approach to the target moment, but a crossing signal fragments at the boundary between worlds. This is a **different failure** from unstable 'where' (Palkia) or 'when' (Dialga). Varnell deduces the need for a dimensional intermediary—Giratina. Don't announce that conclusion in public dialogue. It must be recoverable from lab logs on replay.

## Chapter 8b — Abandoned Rocket Hideout: the historical inventory (DRAFT)

The site stays sealed and deserted. The player receives formal access to recover Rocket-era research on anomalous materials. Inside are mundane records: seized research papers, tower raid inventory and a list of items transferred to League custody.

**Short report:**

PLAYER REPORT (system text): The inventory lists several artifacts that were transferred to League storage after Rocket disbanded.

VARNELL (phone): That's useful. There may be something there we can compare against the records from Silph.

VARNELL: Bring me the full inventory, if you can.

**Optional record, ordinary style:** "Tower structural damage: origin undetermined. Civilian victims recorded; one adult not identified." Do NOT print the child's name or an image of the father. Don't present a staff member who somehow knows the paradox.

## Chapter 9 — Cinnabar: bridge the science without spoiling (DRAFT)

Cinnabar Mansion contains historically relevant but limited science writing, not instructions from Varnell's future self. Readings and malfunctioning hardware hint that stable space and time do not guarantee a safe dimensional connection. Blaine's relocated Seafoam Gym continues to make sense because the island's volcano remains unstable.

**Mansion diary fragment (optional):** "Coordinates remained fixed. The receiving chamber still failed. We assumed an error in the measurements. There was none." The historical diary must be plausibly about ordinary laboratory transfer, not Varnell/Hoopa/Giratina. If this feels too coincidental, cut it and place the fact in later ARIA diagnostic logs instead.

## Chapter 10 — Viridian: Blue's earned respect (DRAFT)

Blue's eighth Gym is his own accomplishment, not a Varnell conspiracy scene. He challenges the player as a skilled former Champion.

**Before battle:**

BLUE: This isn't Oak's laboratory. And Varnell isn't here to tell either of us what to do.

BLUE: You've come this far in Kanto. Show me what you can do.

**After battle:**

BLUE: ...All right. You earned that.

BLUE: When Oak asked you to come here, I thought he was wasting our time.

BLUE: He wasn't.

BLUE: Go beat the League. I want to see how far you take this.

Only AFTER this does Varnell make his private request about a legitimate-looking transfer. Blue cannot already know/suspect the fraud.

## Chapter 10b–11 — Orb authorization and Lance's delayed discovery (DRAFT)

**Proposal, not approved:** Eight Kanto badges and Champion status provide **physical access** to the Hall of Fame archive/vault, but taking a dangerous relic requires additional written research authorization. Varnell has forged a League-approved **transfer order** with a recognizable official designation and an urgent containment pretext. The player uses their own genuine access with apparently valid paperwork. A custodian logs the release; only a separate authorizing-office audit reveals the signature/order wasn't authentic. No one asks the protagonist to knowingly steal.

**Private request, Ch.10b:**

VARNELL: {PLAYER}, there's one final piece of equipment I need the League to release. They recovered it from Rocket years ago.

VARNELL: I have the transfer paperwork prepared. But they won't allow my staff into that vault.

VARNELL: A new Champion can request a supervised release. You're closer to qualifying than anyone I know.

VARNELL: I wouldn't ask if I thought it could wait.

**Vault, after Champion Lance battle (use clerk/custodian NPC and existing storage tiles):**

CUSTODIAN: Champion credentials verified.

CUSTODIAN: Research release form... yes. This item has remained sealed since the Rocket case.

CUSTODIAN: I'll log the transfer. Keep the documentation with the package until it's received.

(Griseous Orb is transferred under plausible official procedure. No alarm, no silent theft.)

**Administrative timing:** the custodian checks form completeness and authentic Champion status; the authorizing office later confirms it did not sign this transfer. Place that later review after the player delivers the Orb, not by unexplained coincidence. Choose exact signatory and review trigger before owner approves dialogue.

## Chapter 11b–12 — The handover and betrayal (DRAFT)

**Sevii facility:** Former Rocket Warehouse has a public, legitimate research area (leased/repurposed, no living Rocket staff) and a sealed concealed containment level built long before Orb arrives. The Orb provides the final locating/anchoring key for Giratina; the room/Hoopa systems and energy bank already exist. The player does not hand over a Ball holding Giratina.

**Handover:**

VARNELL: You got it.

VARNELL: ...Thank you. I haven't slept much. There's more to do than anyone realizes.

VARNELL: I'll take it from here. You've done everything I asked.

(He is controlled but unnaturally anxious. The player leaves.)

**Blue's discovery:** Lance's office flags the fraudulent release, calls Blue because he knows the project and the player. Blue initially blames the player for taking a dangerous Orb, then learns Varnell supplied the paperwork. He is stunned, not already suspicious.

BLUE (PokéNav): {PLAYER}. Where are you?

BLUE: The League's called about something missing from the Hall of Fame vault. The Griseous Orb.

BLUE: They have your name on the transfer.

(After silent player's explanation.)

BLUE: ...Varnell told you to take it?

BLUE: Stay there. I'm coming.

**The return:** Blue and player investigate what looked like a secured storage doorway, now open for staff use. They find the hidden containment level and captured legendary Pokémon, plus the prepared retrieval apparatus. If the schedule is tight, stage Giratina's containment finishing just as they arrive; do not pretend a whole new laboratory was built during a single phone call.

BLUE: That's what all those readings were for.

BLUE: You had us chasing the damage your own machine was causing.

VARNELL: The damage was contained. Most of it.

BLUE: You knew what you were risking.

VARNELL: Yes.

(Only here reveal that Aria was his daughter and what ARIA truly does; the dialogue should allow the player to understand without optional clues.)

BLUE: I trusted you.

VARNELL: I know.

BLUE: After everything you did for me—

VARNELL: I'm sorry, Blue. But she matters more.

**Owner-approved line preserved:** "I'm sorry, Blue. But she matters more." Keep this exact line intact.

**Varnell's moral position:** He is not 'just misunderstood'. He consciously accepted potentially catastrophic global danger for his one child. He doesn't repent when confronted, and he isn't redeemed by his final mistake. Nevertheless his compassion toward Kiyoshi and affection for Blue can have been real.

## Chapter 13 — Varnell final battle -> historic tragedy (DRAFT storyboard and dialogue)

**Battle:** Varnell controls Hoopa, Palkia, Dialga and Giratina (plus agreed nonlegendary team slots pending owner-approved roster). Existing containment/wear-down technology made their capture possible; he is not a legendary-level trainer by strength alone. Battle occurs to interrupt further ARIA operation, not to destroy the world. Winning matters: it disables future recharging and the dangerous wide field; only one **already-charged receiving cycle** remains.

**On return to the overworld:**

(Alarm. Varnell is defeated; some containment systems go dark. One receiving ring remains powered.)

BLUE: Shut it down.

VARNELL: The retrieval cycle is already charged.

BLUE: Then cancel it.

VARNELL: If I interrupt the receiving field now, I can't tell you where that energy will go.

(A stable view of historical Pokémon Tower appears. Aria is alive, with a Rocket member still holding Clefairy elsewhere in the scene. No shakes, flashes of destruction or falling rock.)

**Owner-approved opening line (2026-11-17). Varnell yells as soon as he sees Aria through the stable portal.**

VARNELL: ARIA! My sweet daughter! It’s so good to see you. Please, come here!

ARIA: Dad?

VARNELL: Listen to me. You need to come through. Right now.

ARIA: I can't. They still have Clefairy.

VARNELL: Aria, please.

(Portal displays a few ordinary moments passing in that past corridor. There is no collapse underway. Varnell speaks to Blue.)

VARNELL: I know exactly when that roof came down. We have seconds.

BLUE: Varnell. It's a receiving field. It brings something here. We never made it safe the other way.

VARNELL: She's right there.

BLUE: I know. But if you go through—

VARNELL: If I stay here, she dies again. I have to get her out.

(He jumps. **First** rumble. The past-side tower starts failing. Rocket captor loses Clefairy amid the first falling stones. Clefairy runs back to Aria, reaching her as the roof comes down. Future Varnell is caught beside them. View cuts out. Sound/flash, then silence.)

**Do not say:** "I'm the one who killed her," "that's the unidentified body," "the past has changed," or "the roof was already coming down." Neither Blue nor Varnell should explain the twist. Varnell never knows it.

**Continuity rule:** Clefairy's gender is not specified. Always use its species name or a neutral pronoun in draft dialogue; avoid inventing sex or gender details.

## Chapter 14 — Aftermath, restrained (DRAFT)

**The characters do not understand every physical implication in dialogue**, but they know Varnell's conspiracy and the risk he took. Do not let them celebrate his sacrifice as redemption. Blue is shaken; Oak acknowledges he had trusted Varnell and failed to see what was happening.

OAK: He helped a great many people. I thought I knew him.

OAK: I don't know what to say about what he did.

BLUE: You don't have to.

(Blue to player, later, away from the lab.)

BLUE: I kept thinking you'd get in the way.

BLUE: You were right there when things went wrong. And you stayed.

BLUE: ...Thank you.

**Gameplay hooks:** The captive legendaries are freed after the battle and later obtainable/catchable in post-Kanto events, not handed out as automatic prizes. Hoenn title defense resumes when the Kanto story is finished. Wally appears in calls/cameos, never takes the protagonist role. Optional Red superboss only later.

## Hindsight / spoiler management checklist

| Chapter | Player's simple explanation | Second-run interpretation | What must NOT be confirmed yet |
| --- | --- | --- | --- |
| 0–2 | Anomalies prompt scientific intervention | Varnell's tests/Hoopa were responsible | Capture apparatus and target |
| 3 | Kiyoshi was accidentally lost despite rescue attempts | Proof humans can cross time; reason for Dialga | That Varnell deliberately caused death |
| 3b | ARIA is a stabilization machine | Name honors child / part of retrieval plan | Girl's name, historical target |
| 4 | Rocket killed a child; weird ghost says "Aria" | Clefairy calls its old trainer; unidentified man is future Varnell | Identity of corpse, exact structural cause |
| 5 | Ghost responding to scientific machine | Blue's innocent inference protects Varnell's secret | Big reveals |
| 6 | Silph teleport malfunction | Capturing Palkia caused the strain | Hidden Palkia |
| 7–8 | Grid/time anomalies, Blue becoming cooperative | Dialga containment powered secretly | Hidden Dialga |
| 8b–9 | League archive and historic accident records | Orb location and exact past coordinates | Giratina |
| 10 | Blue acknowledges player's strength | Trust has shifted without Blue's suspicion | Betrayal |
| 10b–11 | Sanctioned transfer in League vault | Fraud arranged by Varnell; League clearance engineered | Fraud until Blue's call |
| 12 | Varnell planned to retrieve daughter, endangered world | Reinterpret all research tasks | That his action killed her |
| 13 | Varnell jumps to rescue child | He caused the old disaster | Corpse identity left implicit |
| 14 | Recovery | Player has survived an emotionally complex catastrophe | No extra explanation |

## Approval & production gates

**Can be drafted without additional game-code authority:** written dialogue drafts and blocking, causal chart, pacing map, scene asset inventory, character knowledge audit, black-box first-run readability test.

**Requires Treston's explicit approval before playable scripts change:** exact Kiyoshi rescue wording; Lavender NPC dialogue; remaining Ch.6–14 dialogue; forged League authorization mechanism/signatory; Clefairy escaping mechanism; final fight team slots; any optional photo/cutscene positions; newly designed maps.

**Not part of this document:** engine changes, Windows executable build/verification, modifying Polish work, final Kanto trainer rosters/encounter tables, new custom sprites. Those are separate project tasks.

**Unresolved factual/logistical questions still needing a decision:** exact custody mechanics (why Clefairy isn't in a closed Poké Ball); young Varnell's forced-exit specifics; the timing of Lance's records check; exact scene coordinates/tile availability; type and form details of Varnell's other party members; mechanics of legendary release and capture; precise age chronology; whether the letter 'ARIA' is publicly treated as an acronym or just the project name. Do not silently select from drafts and claim these are locked.
