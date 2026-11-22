# Project ARIA — Chapter 6 "Saffron: the quarantine" — FULL DRAFT FOR REVIEW

**Status: DRAFT, nothing implemented.** Written 2026-11-21 from the owner's brief (KANTO_PROJECT_ARIA.md, "Owner
decisions, 2026-11-21 (evening)"), the bible's ch. 6 row, the closed-loop audit and the earlier ch. 6 notes in
KANTO_NARRATIVE_SCENES_DRAFT.md. Every line below needs Treston's OK before it goes in the game. Story content —
never show this to Brennan.

## What the chapter has to do (the brief, checked off)

| Owner asked for | How this draft does it |
|---|---|
| No Varnell phone call sending us to Saffron | The hook is a **public radio bulletin** in Celadon. The PokéNav never rings in this chapter. |
| We hear of it independently; Varnell involved naturally, not the source of every objective | The radio, Officer Jenny, a Silph engineer and Blue set every objective. Varnell only appears once, at the end, in person. |
| A believable reason the player can go in when civilians can't | Teleport pads are displacing people (an engineer came out on the roof ten minutes late). Only licensed Kanto trainers on the anomaly team may enter, and the player carries the **Rift Scanner**, the one tool known to lock a tear. |
| Malfunction, quarantine and Master Ball as one mystery | All three point at the same thing in hindsight: something was being **pulled and held** through Silph's teleport arrays (the Palkia capture), and Varnell wanted a ball that can't fail. |
| Varnell subtle; small inconsistencies that only make sense in hindsight | Four planted details (table below), each with an ordinary explanation on the spot. He is warm, helpful and grateful throughout. |
| Blue's gradual development | First time Blue asks for the player's help and splits the work with them; ends with "...You handled your side well." He never suspects Varnell (owner 2026-11-14: no hints from Blue before ch. 12). |
| Don't reveal the truth behind ARIA | Palkia is never named or seen. Nothing says "capture", "daughter" or "portal". |
| Team Rocket is disbanded | Rocket appears only as memory: the lobby plaque, a resident, the president. Silph's FireRed Rocket grunts and Giovanni are removed. |

## The flow (story steps)

`VAR_ARIA_STORY` values are proposals (ch. 5 ends at 20/21).

| Step | Where | Trigger | Sets |
|---|---|---|---|
| 6.1 The bulletin | Celadon, outside | Erika's badge **and** the office visit done (VAR ≥ 20), the next time you're outside in Celadon | 22 |
| 6.2 The checkpoint | Route 7 gate (Celadon → Saffron) | Walking in at 22 | 23 |
| 6.3 Silph lobby | Silph Co. 1F | Entering at 23 | 24 |
| 6.4 The three relays | Silph 3F, 5F, 9F (proposal) | Rift Scanner on each relay | flags |
| 6.5 The array | Silph 11F | All three relays locked | 25 |
| 6.6 The president | Silph 11F, president's office | Talking to him at 25 | 26 |
| 6.7 Quarantine lifted | Outside Silph | Leaving the building at 26 | 27 = **ch. 6 complete** |
| 6.8 Mimi's Gym | Saffron Gym | Open from 27 | badge |

If the player beat Erika before visiting the office, 6.1 fires when they come out of the office. If 5.4 (Blue at the
Snorlax) hasn't happened by 6.1, it is skipped: Blue is in Saffron from then on.

---

## 6.1 Celadon — the bulletin

*(As the player steps outside. A woman stands by the Gym with a portable radio; the radio jingle plays.)*

> RADIO: ...We interrupt this program with a bulletin from Saffron City.
>
> RADIO: Silph Co. has confirmed an equipment failure at its headquarters. Its teleport system is not responding
> normally.
>
> RADIO: As a precaution, the League has closed all four gates into Saffron. Residents near the Silph building are
> being moved to Celadon and Vermilion.
>
> RADIO: League officers are asking registered trainers with anomaly field experience to report to the Route 7
> checkpoint.
>
> WOMAN: Silph again... After the Rocket years, I hoped that building was done making the news.

*(No PokéNav call. The woman stays and repeats her last line if talked to.)*

## 6.2 Route 7 checkpoint — why the player gets in

*(Evacuees in the gatehouse. Officer Jenny and a Silph security guard at the Saffron door.)*

> EVACUEE: They told us to leave everything. Something about the pads sending people to the wrong floors...
>
> JENNY: Hold it. Saffron's closed. ...You're a registered challenger? Let me see your pass.
>
> JENNY: Kanto badges... anomaly team... Huh.
>
> JENNY: You're already on the access list. Filed this morning.
>
> JENNY: ...Before the first alarm went off.
>
> SILPH GUARD: That'll be Dr. Varnell's office. They register their field people ahead of time for anything near
> the Silph arrays.
>
> JENNY: ...Fine. You're cleared.
>
> SILPH GUARD: Two of our engineers stepped on the same pad on 3F this morning. One came out on 5F.
>
> SILPH GUARD: The other came out on the roof. Ten minutes later.
>
> JENNY: That's why nobody without clearance goes in. The pads are reading like those tears, and you've closed
> tears before. Most people haven't.
>
> JENNY: Stay with the Silph engineers. If a pad sends you somewhere strange, don't step on it again.

## 6.3 Saffron and the Silph lobby — Blue

*(Saffron's streets are empty. Optional: an old man sheltering in the Pokémon Center.)*

> OLD MAN: First the Rockets locked this city in. Now it's a machine. At least the machine doesn't shout.

*(Silph 1F. Optional sign by the door.)*

> PLAQUE: "With thanks to the young trainer who freed this building, and to every employee who stayed." — Silph Co.

*(Blue is by the front desk with a Silph engineer.)*

> BLUE: ...You heard the bulletin too.
>
> BLUE: The readings on Route 16 jumped this morning. I followed them here.
>
> BLUE: Good. You've got the scanner. Varnell only ever gave out one of those.
>
> SILPH ENGINEER: The doors still work. The problem is where they send you.
>
> SILPH ENGINEER: We can send an object across this building a hundred times and have it arrive a fraction off
> every time. That's never happened before.
>
> BLUE: Can you isolate the machines on this floor?
>
> SILPH ENGINEER: We did. Whatever's shifting the destinations isn't coming from inside the building.
>
> BLUE: Then where's it coming from?
>
> SILPH ENGINEER: ...If I knew that, I wouldn't need you two.
>
> SILPH ENGINEER: There are three relays between here and the top floor. If that scanner can lock them like it
> locks a tear, the pads should settle.
>
> SILPH ENGINEER: Here. A Card Key. The emergency doors are sealed shut until the system's stable.
>
> BLUE: Then we split it. {PLAYER}, you take the relays. I'll work the control room and keep the pads talking to
> you.
>
> BLUE: ...Don't get yourself sent to the roof.

*(Optional, a second engineer in the lobby.)*

> ENGINEER: Dr. Varnell's team put a new receiving array up on 11F last month, for the anomaly study.
>
> ENGINEER: It's been humming ever since. Nobody here's allowed to touch it.

## 6.4 The three relays

*(The building plays as FireRed's Silph Co. maze, with Rocket removed; see Gameplay notes. Each relay is a machine
in a sealed room. Using it:)*

> The Rift Scanner locks onto the relay... The pads on this floor stop humming.

*(Blue over the building's intercom after each one. It's the PA system, not the PokéNav.)*

> BLUE (intercom): That's one. The pads on 3F just came back. Keep going.
>
> BLUE (intercom): Two. Whatever you're doing, it's working.
>
> BLUE (intercom): That's all three... but 11F is still pulling. Get up there.

*(Optional stranded staff, for example a scientist on 5F.)*

> SCIENTIST: I've been stuck in here since sunrise. Every pad I try puts me right back in this room.

## 6.5 11F — the array

*(A big machine fills the west room. The hum gets louder as the player walks up; screen shake. Rift Scanner:)*

> The Rift Scanner beeps faster and faster...

*(White flash. Silence. The hum is gone. Optional terminal beside the array:)*

> RECEIVER DRIFT: ELIMINATED.
> SECONDARY LOAD: ROUTED OFF-SITE.

*(Blue comes up. Then Varnell steps off a pad with two of his staff, and the lobby engineer follows.)*

> VARNELL: {PLAYER}! Blue! You're both all right.
>
> BLUE: You got here fast.
>
> VARNELL: I was on my way the moment I heard.
>
> SILPH ENGINEER: Doctor, how did you know to come to eleven? We hadn't put out a floor yet.
>
> VARNELL: ......
>
> VARNELL: It's my array. Where else would it be?
>
> SILPH ENGINEER: ...Fair enough.
>
> VARNELL: *(to the array, quietly)* ...It held. It all held.
>
> SILPH ENGINEER: We'll need the array's logs for the incident report.
>
> VARNELL: Of course. I'll take the drives back to the lab and send you a copy tonight.
>
> VARNELL: {PLAYER}. You kept a great many people safe today. Thank you.
>
> VARNELL: The president will want to see you. I have to get these readings home.

*(Varnell and his staff leave with the drives.)*

> BLUE: He's been like that all week. Barely sleeps.
>
> BLUE: ......
>
> BLUE: They made it sound like the whole building was going to fold in on itself.
>
> BLUE: ...You handled your side well.

## 6.6 The president — the Master Ball

> PRESIDENT: So you're the trainer the League sent in. I don't know how to thank you.
>
> PRESIDENT: The last time this building was sealed, a trainer about your age walked in and set it right too.
> Silph seems to need saving every few years.
>
> PRESIDENT: Please, take this. A Master Ball, the finest ball we've ever made. It will catch any Pokémon without
> fail.
>
> *({PLAYER} received a Master Ball!)*
>
> PRESIDENT: We stopped making them after the Rocket years. A ball that can't fail is a dangerous thing in the wrong
> hands.
>
> PRESIDENT: Funny... your Dr. Varnell asked us for the Master Ball plans last spring. For containment research, he
> said.
>
> PRESIDENT: We said no, of course. I hope he understood.

## 6.7 The quarantine lifts

*(Outside Silph. Jenny and Blue.)*

> JENNY: The pads are stable and the readings are flat. We're lifting the quarantine.
>
> JENNY: Saffron's open again. That's on you two.
>
> BLUE: I'm heading to Fuchsia. The Safari Zone's been giving off strange readings for a week.
>
> BLUE: ...I'll let you know if it turns into anything.

*(Blue leaves. All four Saffron gates open. Mimi's Gym, the Dojo and the shops reopen.)*

## 6.8 Mimi's Gym (Normal)

> MIMI: Hi! I'm Mimi! I copy people. Voices, walks, faces... sometimes Pokémon too.
>
> MIMI: I used to copy trainers for fun. Then I started copying how they win.
>
> MIMI: Sabrina left me this Gym when the League called her up. Let's see if you can tell which parts of me are
> really me!
>
> *(Defeated)* MIMI: Wow... I couldn't copy that. Not even a little.

**Optional, a stronger clue. Keep or cut** (talking to Mimi after the badge):

> MIMI: Want to see my newest one? There was a man at the Silph doors this morning, at sunrise. Before any of the
> alarms.
>
> MIMI: *(in a low voice)* "Not yet... not yet..."
>
> MIMI: Then he looked at his watch and walked off. Weird, right?

---

## The planted inconsistencies (only make sense in hindsight)

| # | What the player sees | Ordinary explanation on the spot | What it really means |
|---|---|---|---|
| 1 | The player's Saffron pass was **filed before the first alarm** (6.2) | Guard: Varnell's office pre-registers its field people | He knew the containment would strain Silph's arrays that morning |
| 2 | Varnell goes **straight to 11F** before anyone named the floor (6.5) | "It's my array. Where else would it be?" | He knew exactly what was pulling |
| 3 | "...It held. It all held." and he **takes the logs**, promising a copy "tonight" (6.5) | Relief; he's exhausted ("barely sleeps", Blue) | The capture held. The copy never arrives (a Silph NPC in a later chapter can mention it). |
| 4 | He asked Silph for the **Master Ball plans** for "containment research" (6.6) | Research curiosity; Silph said no | He needs to catch Pokémon he can't beat, which is why the wear-down chambers exist |
| 5 *(optional)* | Mimi copies a man at the Silph doors **at sunrise**: "Not yet..." (6.8) | A random passer-by | He was there before it started |

**Recommendation:** use 1–4. They're spread across four different characters, and each comes with a reasonable
excuse. Mimi's (5) is the strongest. Add it only if you want suspicion to grow faster before ch. 7. The terminal's
"SECONDARY LOAD: ROUTED OFF-SITE" is optional hindsight text for players who read terminals.

Blue never questions Varnell in any of these. The Silph engineer asks the floor question, not Blue, to keep your
2026-11-14 rule ("no hints from Blue before ch. 12"). Blue's "he's been like that all week" is worry, not doubt, and
it sets up Varnell's ch. 12 line "up a week straight trying to get to the bottom of this."

## Gameplay notes (how it would be built)

- **Silph Co. without Rocket:** FireRed's floors are full of Rocket grunts, Giovanni and a rival battle. All of
  that goes. Proposal: a few Silph security staff and scientists as trainers ("containment drill: show me your team
  can handle this before you go up"), and displaced Pokémon from other regions loose on some floors, consistent
  with the tears.
- **The maze:** reuse FireRed's teleport pads and Card Key doors. Before a relay is locked, its floor's pads send
  you to "wrong" destinations (still a fixed, learnable layout, not random). Locking a relay restores FireRed's
  normal pad links on those floors.
- **Lapras:** in FireRed a Silph employee gives you a Lapras for beating Rocket. Keep it as a thank-you from a
  stranded scientist, or drop it? (Question below.)
- **Saffron gates:** anomaly cordon before ch. 6 (as the §4 road-block table already says), quarantine during
  ch. 6 (only the Route 7 checkpoint lets you through, at step 23), all open from 27.
- **Fuchsia early (owner 2026-11-21):** Fuchsia stays reachable early by Routes 12–15. Cycling Road stays closed
  for repairs until ch. 7 (§4 table). Until ch. 6 is complete:
  > GYM GUIDE (Fuchsia Gym door): Lady Janine's away on League business. She'll be back once things in Saffron
  > settle down.

  Saffron is cordoned from the start of Kanto, so this line makes sense at any point before ch. 6 ends. No Blue,
  Safari report or other ch. 7 dialogue appears in Fuchsia until VAR ≥ 27.

## Continuity checks done

- **Rocket:** referenced only as the past (plaque, old man, radio woman, president). No active Rocket.
- **Blue:** cold at first ("You've got the scanner", which also carries a little envy), then trusts the player with
  half the job, then the first real compliment. No suspicion of Varnell. He doesn't know Aria's name.
- **Varnell:** warm, grateful, competent. The engineer's question and the president's remark both get ordinary
  answers. No villain lines.
- **ARIA's truth:** Palkia unseen and unnamed. "Containment" appears once, in the president's line, which is about
  the Master Ball.
- **The drawing in his office (ch. 5)** is not mentioned here. It stays an optional clue.
- **Mimi:** Normal type, older than FireRed's Copycat girl (S-COPYCAT); Sabrina's promotion explained in her intro.

## Questions for you

1. **Ch. 6 complete:** is that the quarantine lifting (step 27), or Mimi's badge? It decides when Fuchsia's Gym
   opens.
2. **Mimi's sunrise clue (5):** keep or cut?
3. **Lapras** from a grateful Silph scientist: keep or cut?
4. **Silph trainers:** Silph staff "containment drill" battles plus displaced Pokémon from other regions, OK? Or
   something else?
5. **Mimi's badge** keeps FireRed's name (Marsh Badge), or does it get a new one?
