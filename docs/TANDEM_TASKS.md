# Project Tandem — online multiplayer (Brennan)

**Status: planning.** Nothing implemented yet. Named by Brennan 2026-11-26 23:36 ("Project Tandem"). Treston said it is
okay (Brennan, 2026-11-26 23:25: "i received an email from PokeMMO and Pokemon Nexus and Treston saying it is okay, i
have their full blessing"). Work order (Brennan 23:36): Polish's Jagged Pass list (his 22:09/22:59 lists) is finished
first, then **T0, the audit**. Branch: `tandem` (made from `polish` when T0 starts; merged back as milestones land).
Owner decisions also go to `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md`. No Kanto story in this file (Brennan reads it).

## What it is (Brennan's brief, 2026-11-26 23:15, from the plan he wrote with ChatGPT)

- One player hosts a multiplayer world from their PC; friends join through private invitations.
- Players see each other moving around the same Hoenn overworld in real time.
- Players can look at each other's player card, challenge each other to Pokémon battles and trade Pokémon.
- Each player's progress is kept in a cloud save.
- Later: a separate launcher that updates the game when opened.
- **First milestone: two players walking around the same Hoenn map and seeing each other in real time.**
- Rules he set: audit the code first and present a plan before coding; no engine rewrite; never modify or regress the
  3D renderer, voxel terrain, pixel-art filtering, lighting, camera or map geometry; single-player keeps working as
  it is; don't invent APIs or assume features that don't exist; name the risks before implementing.
- Standing rules that apply too: old GBA saves must still import (AGENTS.md); it is one game with Kanto (ARIA), so it
  must work in Kanto maps as well.

## Facts from the code (checked 2026-11-26, session 014PwRDj)

- The game is the pokeemerald-expansion C code, built for Windows by `Makefile.pc` (mingw, SDL2); `main` is in
  `src/platform/sdl2.c` (also `src/platform/win32.c`).
- Emerald's own link features are all in the build but switched off on PC: `IsWirelessAdapterConnected()` returns FALSE
  under `PORTABLE` (`src/link.c`); `HandleLinkConnection`, `Task_InitUnionRoom` and the wireless (`rfu_*`) functions
  are stubs (`src/platform/stub.c`). The code itself is there: `link.c`, `link_rfu_2.c`, `union_room.c`, `cable_club.c`,
  `trade.c`, `battle_controller_link_opponent.c`, `battle_controller_link_partner.c`, the link Trainer Card.
- No networking library in the PC build (no sockets in `src/platform/`).
- The save is the GBA-style flash save, written by `src/platform/agb_flash_pc.c` to `WallysJourney.sav`.
- pokeemerald-expansion links (trades, battles) with other expansion builds, not with the official games.

## What other projects did (research 2026-11-26, sources at the end)

- **PokeMMO**: modified GBA/DS emulation cores under a persistent server; players supply their own ROMs; progress,
  items and Pokémon are kept on the server against an account; a login server hands the client a game server; a
  launcher; native client since 2026. A central server owns everything - not a friend-hosted game.
- **PokeNexus** (formerly Pokémon Planet): a Flash browser MMO rebuilt in Unity (2023), browser, Android, Windows, Linux,
  its own central server. Fights grinding bots (a public-MMO problem).
- **Pokémon Emerald Online** (browser MMO built on Emerald): rejects modified games - multiplayer needs identical game
  data; players see each other, trade and battle anywhere; other players' cycling/surfing/fishing animations limited
  because they caused map problems; a shared clock for time-based events; no co-op story, because Emerald is built
  around one player.
- **Emerald Rogue** (pokeemerald-expansion): host/join at an in-game desk; a companion app talks to the emulator through
  a script that reads and writes the game's memory (its own messages, not the link hardware).
- **EmeraldRecomp** (Emerald natively on PC): two-player netplay over a virtual Link Cable or Wireless Adapter - each
  peer simulates both games, inputs exchanged (delay-sync or rollback); matching builds required; cable play-tested,
  wireless trades/battles not yet confirmed.
- **sm64coopdx** (Super Mario 64 decomp PC port): friend hosting, direct UDP or a relay (CoopNet) for players whose
  routers block direct connections.
- **Pokémon Showdown**: the server decides the battle, the client only shows it.

## Proposals (Claude's, NOT yet decided by Brennan - ask before building on them)

1. Share the world, not the story: each player keeps their own story and save; friends see each other in Hoenn.
2. Same build required: the game checks it when someone joins (the launcher keeps everyone current).
3. While connected, the host's clock for everyone (seasons, day/night).
4. Walking and running first; surfing, biking, fishing later.
5. Friends on the neighbouring maps the 3D view shows are drawn too (world coordinates, not map ids).
6. Friends drawn as characters the game's collision and scripts ignore; the 3D renderer draws them as it draws any
   character (no renderer change).
7. Trades and battles: first try the game's own link code over the network, with an early trade test; plan B, the
   game's own messages (Emerald Rogue's way). Battles: Emerald's link battles run the same battle on both games, each
   side's choices passed over the link (TANDEM_AUDIT.md section 4). (T0, 2026-11-27: the
   CABLE layer, not the wireless one - one 8-number exchange per frame against a whole radio library; TANDEM_AUDIT.md section 8.)
8. Connections: direct (LAN, direct IP, or a private network such as Tailscale for testing), then a relay fallback.
9. Saves: back up and sync the existing save file first; accounts only if really needed.
10. Keep it private and invite-only (no public servers).

## Tasks

| Status | Task | Notes |
|---|---|---|
| [x] | **T0** Architecture audit (read-only) + implementation plan for Brennan's review - **done 2026-11-27: `docs/TANDEM_AUDIT.md`** (and Brennan's doc https://claude.ai/artifact/BHzoa527o1EWzCgtTNyVGN, for his comments); waiting for his answers to its 7 questions before T1 | ChatGPT's 10 audit points: build/entry point; movement, coordinates, collision, animation; map loading/transitions/ids; battle architecture; party/inventory/save structures; save/load; rendering and how to keep networking out of it; existing link code; host/relay networking; cloud saves. Exact files and functions per phase. After Polish's Jagged Pass list. |
| [ ] | **T1** Milestone 1: two players walking on the same map, seeing each other in real time | 2D and 3D; neighbouring maps |
| [ ] | T2 Same-build check, host clock | |
| [ ] | T3 A trade through the game's own link code over the network | the lag test |
| [ ] | T4 Battles between players | |
| [ ] | T5 Private invitations and a relay | |
| [ ] | T6 Cloud saves | |
| [ ] | T7 Launcher that updates the game | |

## Brennan's answers to the T0 questions (2026-11-27 02:55, typed into the audit doc)

1. Friends walk through each other (they never block a door). (The doc's suggested answer; he left it.)
2. Fast-forward: "Yes while walking, biking, surfing, off during trades and battles with another player."
3. Players at once: "3 first, we can shoot for more players at a later date once it works flawlessly."
4. Trades and battles: "anywhere by facing a friend and going through an option menu of trade, playercard, battle,
   or team up" (no Cable Club trip).
5. Connecting: "lets try to stick to a free tool to connect." (a free private network such as Tailscale; no paid
   relay)
6. Cloud saves: a synced folder is enough to start - "google drive most likely the best option."
7. The host's clock drives seasons and day/night for everyone while connected: "Yes, from proposal 3."
8. His new ask: "Can you add in multiplayer to team up and it do double battles except for story battles and gym
   battles." Answered in the doc (comment, 2026-11-27): yes - the code has two linked players against computer
   trainers (the Battle Tower's link multi battles, BATTLE_TYPE_TOWER_LINK_MULTI), a linked partner's battler
   (battle_controller_link_partner.c) and wild double battles (WE_DOUBLE_WILD_*); a regular trainer battle while
   walking together becomes a double battle, each player controlling their own Pokemon; story and Gym battles stay
   one-player. A step of its own, after battles between players. Asked him: wild Pokemon too, or trainers only?
   Story battles = rival, Team Aqua/Magma, Elite Four, Champion - any others?

## Open questions for Brennan

- Team-up battles: wild Pokemon too, or trainers only? Which battles count as story battles besides the rival, Team
  Aqua/Magma, the Elite Four and the Champion?
- Different story progress: each player sees their own world (proposal 1)?
- Trades: any check on edited Pokémon coming in?

## Sources (2026-11-26)

- PokeMMO: https://tech-insider.org/pokemmo-setup-guide-2026/ ; https://flathub.org/apps/com.pokemmo.PokeMMO ;
  https://github.com/openmmo-org/OpenMMO ; https://www.pikammo.fr/Pokemmo/en/news/patch-shiny-wars-2026-refonte-rencontres
- PokeNexus / Pokémon Planet: https://www.mmobomb.com/all-pokemon-mmos-you-can-play-free-right-now ;
  https://x.com/PokePlanetMMO ; https://lutris.net/games/pokenexus/ ; https://www.indiedb.com/games/pokemon-planet
- Pokémon Emerald Online: https://www.generationamiga.com/2026/08/06/pokemon-emerald-online-turns-the-gba-classic-into-a-browser-mmo/
- Emerald Rogue: https://github.com/var-arora/emerald-rogue-assistant ; https://pastebin.com/WuT4VvsB
- EmeraldRecomp: https://github.com/mstan/EmeraldRecomp
- sm64coopdx: https://deepwiki.com/coop-deluxe/sm64coopdx
- Pokémon Showdown: https://www.smogon.com/forums/goto/post?id=9534800
- pokeemerald-expansion linking: https://rh-hideout.github.io/pokeemerald-expansion/print.html
