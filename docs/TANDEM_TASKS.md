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
(Steps renumbered 2026-11-27 to the audit's plan, TANDEM_AUDIT.md "The plan, step by step".)

| [x] | **T1** Connect two PCs: network module, host/join, build check - **done 2026-11-27** | `src/platform/tandem_net.c/.h` (own thread, TCP, up to 3 players, build = git commit refused if different, hello + pings, log `WallysJourney_tandem.log`, status in the window title); `sdl2.c` (settings `tandemHost`/`tandemJoin`/`tandemName`, start/stop, title); `Makefile.pc` (plain-C rule, build header, `-lws2_32`). Tested: host + 2 games + test clients on one machine (`tools/tandem/`): joins, names, pings 0-3 ms, full game refused (joiner retries), wrong build refused, leaving/rejoining; not configured: 6 views pixel for pixel the same, settings file the same, no log. How to connect: `docs/TANDEM_HOWTO.md`. Not yet tried between two real PCs (Brennan). |
| [x] | **T2** (first milestone) See each other on the same map - **done 2026-11-27** | `src/tandem_overworld.c` + `include/tandem_overworld.h`: each friend on my map is a "ghost" person (local ids 245-247, only in the spare object slots 16-63, never saved), walked with the game's own movement actions at the friend's speed (walk / run / bike / surf); more than 6 steps behind, it jumps to the latest. `tandem_net.c` protocol 2: STATE message (map, x, y, facing, speed, graphics, elevation) sent when it changes and once a second; the host passes each friend's on to the others. Hooks: `overworld.c` OverworldBasic (one call, returns at once when not connected), `event_object_movement.c` GetObjectObjectCollidesWith (ghosts block nobody and nobody blocks them: Brennan "walk through each other"), `wj_limits.c` (ghost slots). Tested on one machine, two games: 2D and 3D, both screens, standing and walking, walking through each other. Debug: `WJPC_TANDEM_DEBUG=1` (ghost log to the console), `WJPC_TANDEM_GHOST_GFX=<id>` (draw friends as another character, for tests). Note for testing: the same build gives different pictures run to run (people wander at random, cloud shadows), so single-player was checked by the code path (nothing runs unless connected; the two collision lines only match ghost ids), not pixel for pixel. Lesson: a test friend whose script ends leaves the game, and their ghost goes with it - keep test friends connected for the whole run. The friend's following Pokemon (Brennan 15:05: "yes we do want the pokemon to follow on everyones screen"): protocol 3 adds it to STATE (graphics, tile, facing; none while it is hidden - in its ball, surfing); drawn as a second ghost (local ids 232-234) and stepped with its trainer. Found on the way: `SpawnSpecialObjectEventParameterized` (event_object_movement.c, not ours) fills only some of its template - script, flag and the rest are left as whatever was on the stack - and the friend's Pokemon failed to appear after a map change; Tandem now spawns from a template with every field set (SpawnGhost). The game's other two callers (field_specials.c ~line 622, the Berry Blender players, and ~line 1365, a boy) may hit the same thing: left for Treston. |
| [x] | **T3** Across map changes and neighbours; doors, warps; Kanto - **done 2026-11-27** | each player's own world (Brennan 13:39: yes). `src/tandem_overworld.c`: a friend on my map or on a map beside it (`gMapHeader.connections`: north / south / east / west, with the offset, as `fieldmap.c` joins the maps) is placed in my map's coordinates (FriendMapShift / PlaceOnMyMap) - walking across a border is one step on either screen; when I cross, the game moves the ghosts with its own people. Ghosts are made only inside the area where the game keeps its people (RemoveObjectEventIfOutsideView's box, with the PC's wide-view padding), as the game would remove them outside it. Through a door, Fly or any warp to a map that is neither mine nor beside it, the ghost goes; it comes back when they come out. Tested (two games, 2D and 3D): friend in Oldale seen from Route 101 across the border, walked onto Route 101 and back, into the Oldale Pokemon Center (gone) and out (back at the door); host walked from Route 101 into Oldale and still saw the friend; and the same at a border between two maps of the other region (map group 37). Also fixed: `Makefile.pc` FORCE was not phony, so make took it for a missing in-between file and never refreshed the build id (every build said 01349efa) - now `.PHONY: FORCE`. Debug (`WJPC_TANDEM_DEBUG=1`): logs each friend's map changes and when a friend is outside the area round me. |
| [x] | **T4** Player cards; the face-a-friend menu (trade, player card, battle, team up) - **done 2026-11-27** | A on a friend (`field_control_avatar.c` GetInteractedObjectEventScript -> TandemOverworld_GhostScript) runs `data/scripts/tandem.inc`: "What would you like to do with NAME?" and a dynmultichoice Trade / Player Card / Battle / Team Up / Cancel; Trade, Battle and Team Up say "That isn't ready yet. It's coming in a later update!" until T5/T6. Cards: each player sends their card (TrainerCard_GenerateCardForLinkPlayer, the one the Cable Club swaps) every 5 seconds, `tandem_net.c` MSG_CARD (protocol 4); Player Card shows it with ShowTrainerCardInLink (specials Tandem_BufferFriendName / Tandem_ShowFriendCard). The link card shows no badges and shows the profile phrase on that line; a friend's card shows their Hoenn badges (sent in the card's spare byte) in place of the phrase, as your own card does (`trainer_card.c`, 3 small hooks). Outfits (found while testing): the game drew every Brendan/May sprite - so every friend - in MY outfit. Now each friend has graphics ids of their own (0xE00 + 16 x slot, `event_object_movement.c` GetObjectEventGraphicsInfo hook) and palettes of their own (OBJ_EVENT_PAL_TAG_TANDEM_FRIEND + slot); the outfit (number, skin, two colours) goes with each place (protocol 5) and `wj_outfit.c` makes their colours (WJ_OutfitOwPaletteFor) and, on their card, their outfit picture (WJ_OutfitShowFriendFront; mine put back with WJ_OutfitRefresh). Tested in 2D and 3D: menu, card front (outfit, badges) and back, Trade message, both screens showing the other's outfit. Test aid: `WJPC_TANDEM_TEST_OUTFIT=outfit,skin,colour1,colour2`. |
| [x] | **S1** The server program, WallysJourneyServer.exe - **done 2026-11-27** (Brennan 18:20: "click a .exe and launch the server"; 18:23 answers: server first; "initial invite code, no password required after") | `src/platform/server/tandem_server.c` (console window: invite code, how to join, who is playing; `WallysJourneyServer.cfg` port / invite / name, `_players.txt` the keys, `.log`) + `tandem_net.c` MODE_SERVER (a hub with no player: friends in slots 0-2) and protocol 6 (the hello carries an invite code and a key; a new player let in by the code gets a key, which the game keeps in its settings - `tandemKey=`, the code dropped - so no code after; a known key from a second PC at once is refused). The game plays alone while the server can't be reached and joins by itself when it is back (as before). `Makefile.pc` builds it next to the game (same build id); `make_package.sh` puts it in the zip. Tested: server started after two games - both joined with the code; wrong / missing code refused; key-only rejoin let in; keys saved in both games' settings. |
| [x] | **S1b** The server on a free cloud computer (Brennan 18:39: "lets try the oracle cloud") - **server side done 2026-11-27**; his account and computer pending | `WallysJourneyServer-linux` (x86-64, static, one file: `src/platform/server/linux_sdl/SDL.h` stands in for the few SDL calls with POSIX threads; `make -f Makefile.pc WIN=1 tandem-linux-server`) for VM.Standard.E2.1.Micro (no ARM build: no arm64 compiler here); `tools/tandem/cloud/setup-server.sh` (user `wallys`, /opt/wallys-server, the port in iptables + netfilter-persistent, a systemd service, prints the invite code and address). Tested here: invite, wrong code, key-only. Steps: docs/TANDEM_HOWTO.md. |
| [ ] | **S2** Saves on the server: backup, and the same save on any PC (Brennan 18:23: both) | each player's save to the server when they connect and when they save; on connect, a newer save on the server than on this PC comes down. When a save was played offline on two PCs (both newer than the server's): ask the player which one to use (Brennan 18:39: "yes ask them which one to use"). Earlier plan (T7) said Google Drive - replaced by the server. |
| [ ] | **S3** Game updates from the server (Brennan 18:23: "push game updates") | the server keeps the newest game; an older game gets it when it connects (a small launcher replaces the files and starts the game). |
| [ ] | T5 Trade test, then trades through the game's own link code | the lag test; trust friends' Pokemon (Brennan 13:39) |
| [ ] | T6 Battles between players; team-up double battles (trainers only, not story/Gym battles) | |
| [ ] | T7 Host's clock for everyone; save backup in a synced folder (Google Drive) | |
| [ ] | T8 Relay (free), invites, launcher that updates the game | |

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

   Brennan 2026-11-27 03:05 (in the doc's thread): "1. only trainers for now 2. those are correct, any turning point
   battles or major ones. battle against your neighbor may as well." So: team-up double battles against regular
   trainers only (no wild ones for now); one-player battles: Gyms, the neighbor (the rival), Team Aqua/Magma, the
   Elite Four, the Champion and any other turning-point or major battle.

Brennan 2026-11-27 13:39: different story progress - each player sees their own world: "yes"; trades, check
edited Pokemon or trust friends: "trust your friends".

## Open questions for Brennan

(none open)

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
