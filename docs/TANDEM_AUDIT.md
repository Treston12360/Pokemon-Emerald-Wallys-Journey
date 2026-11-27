# Project Tandem — T0 audit and plan

2026-11-27, session 014PwRDj, for Brennan's review (he asked 02:26: "Run the audit now to get that complete then work
route 118 after that. so i can overlook the audit while you work on the last fixes."). Read-only: nothing in the game
changed. The same text is in Brennan's doc "Project Tandem — T0 audit and plan"
(https://claude.ai/artifact/BHzoa527o1EWzCgtTNyVGN), where he can comment. Plan and brief: `docs/TANDEM_TASKS.md`.

## Summary

Multiplayer can be added without rewriting the engine. Walking together needs one new piece: a network link plus
"ghost" characters for your friends. Trades, battles and player cards can reuse the game's own link screens.

- **What already helps:** Emerald's trade, battle, trainer card and Cable Club screens are all still in the game, just
  switched off on PC. The overworld can already show up to 4 linked players as characters.
- **What has to be new:** the network connection itself (the PC build has none today), and a way to show friends who
  are walking anywhere in Hoenn, not only in one link room.
- **The 3D view needs no changes.** A friend added as a game character is drawn in 2D and 3D automatically.
- **The biggest risk** is trades and battles over the internet: the game's link code expects every game to move in
  exact step, frame by frame. That gets tested early, before trades are built on it.

Recommended order: connect two PCs → see each other on the same map → across map changes → player cards → trades →
battles → shared clock and save backup → relay, invites and launcher.

## 1. How the game starts and runs each frame

The game runs on its own thread, one GBA frame at a time (about 60 a second); the window and drawing run on the main
thread. A network connection gets a third thread and never makes the game thread wait.

| Where | What it does |
| --- | --- |
| `src/platform/sdl2.c` `main()` | Opens the window, reads settings and save, starts the game thread: `SDL_CreateThread(DoMain)` |
| `src/main.c` `AgbMain()` → `AgbMainLoop()` | Each frame: read keys, run the link step, run the screen's callbacks, wait for the frame to be shown |
| `src/main.c` `UpdateLinkAndCallCallbacks()` | Calls `HandleLinkConnection()` first; on PC a stub that does nothing (`src/platform/stub.c`) |
| `src/platform/sdl2.c` `VBlankIntrWait()` | Hands each finished frame to the main thread (240 fps in-between pictures, fast-forward) |

- The frame loop already has hooks for linked play (`Overworld_SendKeysToLinkIsRunning` / `RecvKeysFromLink`): the
  Cable Club rooms use them.
- Network messages are read once per frame at one fixed point in the game thread.
- Fast-forward (up to 5x) and the frame rate matter during trades and battles (both games in step): see Risks.

## 2. Moving, collision and animation

Everyone on screen is an "object event"; the PC build has 48 spare slots. A friend can be one, moved with the game's
own walking animations, with one rule so they never block you.

| Where | What it does |
| --- | --- |
| `gObjectEvents[64]` (`include/global.fieldmap.h`, `struct ObjectEvent`) | Every character: `currentCoords`, facing, elevation, graphics, its map. Slots 0-15 saved; 16-63 PC-only spares |
| `src/field_player_avatar.c` `PlayerStep()` | Key presses into steps: walking, running, biking, surfing |
| `src/event_object_movement.c` movement actions | Walk, run, jump, face: the game's own animations; a ghost can be driven with these |
| `src/event_object_movement.c` `GetObjectObjectCollidesWith()` | Characters block each other. Followers are exempt (`OBJ_EVENT_ID_FOLLOWER`, `FollowerNPC_IsCollisionExempt`); ghosts need the same |
| `src/overworld.c` `gLinkPlayerObjectEvents[4]`, `SpawnLinkPlayerObjectEvent()`, `CreateLinkPlayerSprite()` | The Cable Club's linked players (up to 4), drawn and animated as characters |

The Cable Club's way works in one room only (every game runs the same frames with everyone's keys). Friends anywhere in
Hoenn need their own positions sent; the plan reuses how linked players are drawn and animated, not the lockstep.

## 3. Maps and moving between them

Every map has a fixed id (group, number), the same on every copy of the same build; Kanto's maps are in the same table.
A friend's position is sent as "this map, this tile" and placed on your screen.

| Where | What it does |
| --- | --- |
| `gSaveBlock1Ptr->location` (`struct WarpData`) | Where the player is |
| `src/overworld.c` `SetWarpDestination()`, `WarpIntoMap()`, `CB2_LoadMap` | Doors, stairs, Fly, warps: a full map load |
| `src/overworld.c` `LoadMapFromCameraTransition()` | Walking across a route's edge into the next map |
| `src/field_camera.c` `CameraUpdate()` | The camera following the player |
| `include/global.fieldmap.h` `IN_KANTO_MAP` | Kanto maps are marked by their layout (`isFrlg`), not a separate table |
| `src/platform/wide_view.c` (neighbours), `voxel_entities.c` `GatherStandIns()` | The PC draws the maps round the current one, with their people, placed by the connections |

- The game keeps only the current map's characters; a friend on the next route is placed through the connection, as
  the wide view places that map's people.
- Through a door or warp, that friend's ghost leaves your screen; it comes back on the same or a neighbouring map.
- Inside buildings each player has their own copy of every room; two friends in the same Pokémon Center see each other.

## 4. Battles

Link battles are complete in the code: both games run the same battle, passing each side's choices over the link layer.
If that layer works over the network, battles come with it. Hardest to get exactly right, so after trades.

| Where | What it does |
| --- | --- |
| `src/cable_club.c` `TryBattleLinkup()`, `Task_StartWiredCableClubBattle()`, `CB2_ReturnFromCableClubBattle()` | The Pokémon Center 2F Colosseum |
| `src/battle_main.c` `CB2_HandleStartBattle()` | Swaps the battle header and both parties (`SendBlock`, `GetBlockReceivedStatus`) |
| `src/battle_controllers.c` `PrepareBufferDataTransferLink()`, `Task_HandleSendLinkBuffersData()`, `Task_HandleCopyReceivedLinkBuffersData()` | Every battle command goes through these link buffers |
| `src/battle_controller_link_opponent.c`, `battle_controller_link_partner.c` | Play the other player's side |
| `gBattleTypeFlags & BATTLE_TYPE_LINK` | Marks a linked battle |

- Both games must work out every turn the same way; full test battles both ways before calling it done.
- Tera, Dynamax and Z-moves go through the same buffers; each needs checking in a linked battle on PC (none tried).

## 5. Party, bag and save data

| Where | What it holds |
| --- | --- |
| `gPlayerParty` (`gSaveBlock1Ptr->playerParty` saved; `src/load_save.c` `SavePlayerParty()` / `LoadPlayerParty()`) | The party |
| `gSaveBlock1Ptr` `struct Bag` (`SavePlayerBag()` / `LoadPlayerBag()`), `src/wally_bag.c` | The bag (and Wally's Journey's unlimited bag) |
| `gSaveBlock1Ptr` | Location, flags, vars: story progress |
| `gSaveBlock2Ptr` | Name, trainer ID, options, Pokédex, play time, clock offset |
| `gPokemonStoragePtr` | PC boxes |
| `gSaveBlockPC` (`include/global.h`) | PC-only data after the GBA part (Kanto flags/vars...); new fields at its end only |
| `src/trainer_card.c` `TrainerCard_GenerateCardForLinkPlayer()`, `CopyTrainerCardData()`, `ShowTrainerCardInLink()` | A card to send over the link; show a received one |

Each player keeps their own save and story; nothing of another player is written into your save except a trade.

## 6. Saving and loading

| Where | What it does |
| --- | --- |
| `src/platform/sdl2.c` `ReadSaveFile(sSavePath)` | `WallysJourney.sav` beside the exe, or the path in `WJPC_SAVE` |
| `src/save.c` `TrySavingData()`, `HandleSavingData()`, `LoadGameSave()` | GBA save sectors |
| `src/platform/agb_flash_pc.c` | The flash chip's stand-in: sectors to the file |
| `src/event_data.c` `SaveBlockPC_Write()` / `SaveBlockPC_Read()` | The PC-only block after the GBA image |
| `src/trade.c` `LinkFullSave_*()` | A link trade saves in a careful order so a cut-off trade leaves both saves whole |

- Old GBA saves keep importing while the 128 KB part is untouched; anything new goes at the end of the PC block.
- A network trade keeps that save order on both PCs; pulling the connection mid-trade must never lose or copy a
  Pokémon (a test, not an assumption).

## 7. The 3D view, and keeping networking out of it

The renderer needs no changes: it draws every active character with a sprite, in 2D and 3D
(`voxel_entities.c`, its loop over `gObjectEvents`; `GatherStandIns()`; `wide_view.c`).

- No multiplayer code in `src/platform/voxel3d/`. Lighting, camera, terrain and map shapes stay as they are.
- The network thread never touches characters or drawing; it queues messages, the game thread applies them once a frame.
- Not connected = no multiplayer code runs: single-player pixel for pixel the same (the picture test checks it).
- Name tags over friends' heads would be a renderer change: left out unless Brennan asks.

## 8. Emerald's own link code, switched off on PC

| | Cable | Wireless |
| --- | --- | --- |
| Code | `src/link.c` `LinkMain1()` / `LinkMain2()` | `src/link_rfu_2.c` + `src/librfu_*.c` |
| What it swaps | 8 numbers per player every frame (`gSendCmd`, `gRecvCmds`, `CMD_LENGTH` 8); blocks (`SendBlock`, `GetBlockReceivedStatus`) | Packets through a radio protocol, dozens of library calls |
| Players | Up to 4 (`MAX_LINK_PLAYERS`) | Up to 5 (`MAX_RFU_PLAYERS`) |
| Used by | Cable Club trades/battles, Record Corner, the linked overworld rooms | Union Room, wireless trades/battles |
| On PC today | `HandleLinkConnection()` a stub doing nothing (`src/platform/stub.c`) | `IsWirelessAdapterConnected()` returns FALSE (`src/link.c`): never entered (the library is built but talks to GBA hardware) |

Above that layer everything is real: `src/cable_club.c` (`TryTradeLinkup()`, `TryBattleLinkup()`), `src/trade.c`
(`CB2_LinkTrade`), `src/trainer_card.c`, the link battle code, `src/union_room.c`.

**Change to proposal 7 (TANDEM_TASKS.md):** it said wireless first. The cable layer is the better first try: one
exchange per frame of 8 numbers per player, against a whole radio library. A "virtual cable" sends those over the
network (EmeraldRecomp's way). Cost: every game in exact step; over the internet a few frames of delay, and both games
pause when a packet is late - hence the early trade test.

## 9. Hosting and joining

- No networking in the PC build today; Winsock (`ws2_32`) is in llvm-mingw - no new DLL. Same calls on the Linux test
  build. SDL2_net not in the build, not needed.
- A new `src/platform/tandem_net.c` / `.h`, started from `src/platform/sdl2.c`; `-lws2_32` in `Makefile.pc`.
- Positions as small packets (a lost one doesn't matter); trades, battles and cards on a reliable connection.
- Joining checks the build (git commit embedded at build time); a mismatch is refused.
- Connecting: LAN by address → internet by port forward or a private network (Tailscale) for testing → later a relay
  (sm64coopdx's CoopNet way). Private invite codes later, no public server list.
- On a direct connection the host and each friend see each other's internet address; a relay would hide them.

## 10. Cloud saves

| Option | How | Work | Catch |
| --- | --- | --- | --- |
| A. Synced folder | Save in OneDrive/Dropbox/Google Drive; a `WallysJourney.cfg` setting points there (`WJPC_SAVE` exists as an env var) | Small: one setting + a lock | Each player sets up their own folder |
| B. Host keeps backups | Joining sends your save to the host, who keeps the last few | Medium | Only while playing together |
| C. Own server with accounts | Log in; saves go up and down | Large: server, accounts, security | Someone keeps it running |

Recommended A, then B; C only if needed. The save file stays exactly as it is (old GBA saves import); every upload
keeps previous copies.

## The plan, step by step (each finished and tested before the next)

| Step | What | Done when | Files it adds or changes |
| --- | --- | --- | --- |
| T1 | Connect two PCs: network module, host/join, build check | Two PCs connect and swap test messages | New `src/platform/tandem_net.c/.h`; `src/platform/sdl2.c`; `Makefile.pc` |
| T2 (first milestone) | See each other on the same map | Two players walk Littleroot together; single-player pixel test unchanged | New `src/tandem_overworld.c` (ghosts in slots 16-63, the game's own movement actions); one call a frame from `src/overworld.c`; `src/event_object_movement.c` `GetObjectObjectCollidesWith()` |
| T3 | Across map changes and neighbours; doors, warps; Kanto | Walk Route 101 to Oldale while the friend stays | `src/tandem_overworld.c` |
| T4 | Player cards | Both see the other's card | `src/trainer_card.c`; the A-button check on a character (exact place found in T4) |
| T5 | Trade test, then Cable Club trades | Trades both ways; a cut connection mid-trade loses nothing | `src/link.c` (`HandleLinkConnection()` on PC over the network); `src/platform/stub.c`; `cable_club.c` / `trade.c` only if needed |
| T6 | Battles through the same link | Full battles end the same on both screens | Link battle files only where tests show a need |
| T7 | Shared clock (seasons, day/night) and save backup | Same season on both PCs; a save restores from its backup | `src/platform/siirtc_pc.c`; `src/platform/sdl2.c` |
| T8 | Relay, invites, launcher | A friend behind a blocking router joins by invite | Separate programs (relay, launcher); invite codes |

Never changed: `src/platform/voxel3d/`, the 128 KB part of the save, how the game plays when not connected. Every step
claims its files in `docs/WORK_CLAIMS.md` first (shared engine code - Treston's OK where his work is).

## Risks

| Risk | What could go wrong | How the plan handles it |
| --- | --- | --- |
| Lag in trades and battles | Lockstep stalls on a slow connection | Trade test first (T5); plan B: the trade and battle steps as messages, no lockstep |
| Battles out of step | One different roll, two different battles | Full test battles both ways, with Tera/Dynamax/Z-moves |
| Connection lost mid-trade | A Pokémon lost or duplicated | Keep the trade's save order on both PCs; cut the connection at every step in tests |
| Different builds | Different data | Join checks the build |
| Fast-forward | Drags the other game in trades/battles | Off during trades and battles; while walking a friend just moves faster |
| Friends in the way | Blocking doors, triggering trainers or scripts | Ghosts never block, trigger or save |
| Shared code with ARIA | Overworld, characters, saves are Treston's too | Claims first; ask Treston |
| Kanto | Different behaviour in Kanto | Tested in T3; stories stay per player |
| Privacy | Addresses seen on direct connections | Invite-only; relay later |

## Questions for Brennan (the plan's guess in brackets)

1. Friends walk through each other, or block like NPCs? (Walk through.)
2. Fast-forward while walking together? (Yes; off in trades and battles.)
3. How many players? (2 first; the game's code goes to 4.)
4. Trade/battle where: the Pokémon Center 2F Cable Club, or anywhere by facing a friend? (Cable Club first.)
5. Port forward or Tailscale for first tests, relay later? (Yes.)
6. Cloud saves: a synced folder enough to start? (Yes; accounts only if needed.)
7. The host's clock for seasons and day/night while connected? (Yes, proposal 3.)
