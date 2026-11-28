# Project Tandem - how to connect (steps 1-6 and the server: connecting, seeing each other, across maps, player cards, trading, battles, team up, saves on the server)

You see who is connected in the window's title, and `WallysJourney_tandem.log` (next to the game) lists who joined,
the hello messages and the delay. When a friend is on the same map as you, or on the map just across a border
(the next route or town), they show up as a person walking around (walking, running, biking or surfing as they do),
with their following Pokemon; you walk through each other. They disappear when they go through a door or fly
somewhere you can't see, and come back when they come out. Everyone shows in their own outfit.

Face a friend and press A for the menu: Trade, Player Card, Battle, Team Up. Player Card shows their card (their outfit,
money, Pokedex, time, badges).

**Team Up:** Team Up on a friend asks them ("NAME wants to team up!"); yes: you are partners (one partner at a time).
Team Up on your partner again: "Stop teaming up?" (they are told). From then on, when a regular trainer spots you or you
talk to one, and your partner is on your screen (the same map or the one across the border, close enough to see each
other), your partner gets "NAME was spotted by Bug Catcher Rick! Join the battle?" while you see "Calling NAME to
join... (B Button: battle alone)". Yes: a double battle - you and your partner against that trainer, each with your
whole team, each choosing for your own Pokemon. No, busy (in a menu or a battle) or no answer in 15 seconds: you battle
alone, as always. Gym trainers, Gym Leaders, the rival, Team Aqua and Magma, the Elite Four, the Champion and other story
battles always stay one-player. In a team-up there is no Bag and no running (as in any link battle / trainer battle).
Winning: the trainer is beaten and the prize money paid only for the one who was spotted (the partner sees "NAME got
...for winning!"); back on the map, each of you gets your own Pokemon's Exp. Points (by your own game's rules: level
cap, EXP ALL, Exp. Share, Lucky Egg...), new moves (the four-move question) and evolutions. Losing: both pay as for a normal
loss ("You gave ... to the winner") and go back to the Pokemon Center. Your Pokemon keep what happened in the battle (no healing). If the connection breaks during it: "A
Button: Continue" - you carry on where you stood, and the trainer battle didn't count.

**Battles:** Battle asks "Which kind of battle?" - Single or Double (a Double needs two Pokemon that can battle, Eggs
don't count) - then asks the friend ("NAME wants a Single Battle. Will you battle?"). Yes: the game's own link battle
(the Cable Club's). Everyone battles at full health, and afterwards your Pokemon are put back exactly as they were
(no free healing). Running = giving up the match (the friend wins). The game saves after the battle, where you stand.
If the connection breaks during it, "A Button: Continue" puts you back where you stood, with your Pokemon as before.

**Trading:** Trade asks the friend ("Asking NAME to trade... B Button: stop waiting"); they get "NAME wants to trade.
Will you trade?" as soon as they are free in the overworld (in a menu, a battle or a cutscene for more than a few
seconds: "can't trade right now"). Yes: both go to the game's own trade screen (the Cable Club's), pick a Pokemon each,
"Is this trade okay?", the trade, and the game saves; then you can trade again. To leave, both pick Cancel (as at the
Cable Club). You come back where you stood. If the connection breaks during it: "The connection to your friend was
lost. A Button: Continue" - you carry on where you stood (or, if it broke between the swap and the save, from your
last save, so a trade is never half done).

Everyone needs the **same game zip** (the game refuses a different build and says so in the log).

If the server (or host) can't be reached, the game just plays alone, and keeps trying every few seconds in the
background; it joins by itself when the server is back.

## The server (recommended): WallysJourneyServer.exe
Double-click `WallysJourneyServer.exe` (next to the game). A window opens and stays open while people play (close it
to stop the server). It doesn't run the game - it connects up to 3 players to each other. Start your own game too
and join it like everyone else.

The first time, it makes an **invite code** (shown in the window, kept in `WallysJourneyServer.cfg`). A new player
types it once; the server then remembers their PC (it gives the game a key, saved in their `WallysJourney.cfg`), so
they never need it again. The players let in so far are in `WallysJourneyServer_players.txt` (delete a line to forget
one). `WallysJourneyServer.cfg` (change it with the server closed):
```
port=27015
invite=K7Q2MX
name=the server        (up to 15 letters)
```
Windows asks whether WallysJourneyServer.exe may use the network the first time: allow it.

### Everyone (you too): join the server
In `WallysJourney.cfg` (next to the game):
```
tandemJoin=SERVER-ADDRESS:27015
tandemName=YourName
tandemInvite=K7Q2MX      (first time only - the game swaps it for its key)
```
SERVER-ADDRESS:
- **On the server's own PC:** `127.0.0.1`
- **Same house / same Wi-Fi:** the server PC's local address (shown in the server window; Windows: `ipconfig`,
  "IPv4 Address", e.g. 192.168.1.20).
- **Over the internet**, one of:
  - **Port forwarding** (free): in your router's settings, forward TCP port 27015 to the server PC's local address.
    Friends use your public address (whatismyip.com). Check first that your internet company gives you a real
    public address: if the router's own "WAN/Internet IP" differs from whatismyip.com (often 100.64.x.x-100.127.x.x),
    it's shared ("CGNAT") and port forwarding can't work - use one of the others. A free name (e.g. DuckDNS) keeps the
    address the same when your public address changes.
  - **playit.gg** (TCP tunnels need Premium, from $3/month): install the playit program on the server PC, make a TCP
    tunnel to port 27015, friends use the address it gives. No router changes; your home address stays hidden.
  - **Tailscale** (free): everyone installs it and joins your Tailscale network; use the server PC's Tailscale
    address (100.x.x.x).
  - **A free cloud computer** runs the server instead of your PC (Brennan 2026-11-27: Oracle Cloud) - up even when
    your PC is off; no router changes. See below.

### Your save on the server (step S2)
A game that joins **the server** keeps your save there too: a backup, and the same save on any PC. Nothing to set up.
(A game that hosts, or joins a friend's hosting game, doesn't do this - only the server keeps saves.)
- **Every time you save in the game**, the save goes to the server a few seconds later. The server keeps every
  save it replaces too, by date (`WallysJourneyServer_saves`, next to the server - on the cloud computer
  `/opt/wallys-server/WallysJourneyServer_saves`). Nothing there is ever deleted.
- **When the game starts** it checks with the server first (a few seconds, "checking your save on the server" in the
  window's title):
  - the same save on both: play;
  - only this PC's changed (played without the server): it's sent;
  - only the server's changed (played on another PC), or this PC has no save: the server's is taken - this PC's is
    kept next to it as `WallysJourney.sav.before-server-DATE`;
  - **both changed** (played on two PCs without the server): a window asks **which save to play** (name, play time
    and when each was saved). The other is kept (on this PC by date, or on the server by date).
- **The server was off when the game started:** this PC's save is played; when the server comes back the game
  checks again. If the server's save changed in the meantime the game doesn't send over it - the window's title says
  to restart the game to choose.
- **Your save on another PC:** that PC needs your key - copy the `tandemKey=` line from `WallysJourney.cfg` on your
  first PC into `WallysJourney.cfg` on the other one (with `tandemJoin=` and `tandemName=`), then start the game:
  it takes the server's save. (With the invite code instead, the server takes that PC for a new player.)
- `WallysJourney.sav.tandemsync` (next to the save) is which save this PC and the server last agreed on - leave it.

### A free cloud computer (Oracle Cloud)
Step-by-step, with every choice on Oracle's pages: **docs/TANDEM_ORACLE_SETUP.md** (give Brennan that list when he
asks how to set up Oracle).

Files (from the game zip): the `cloud-server` folder - `WallysJourneyServer-linux` (the server for x86-64 Linux),
`setup-server.sh` (`tools/tandem/cloud/`) and `src/` (the server's source with this build's id, for ARM computers). Build: `make -f Makefile.pc WIN=1 tandem-linux-server` after the game (same build id).
1. **Account:** oracle.com/cloud/free - email, phone, and a credit/debit card for identity (not charged unless the
   account is upgraded; no prepaid or virtual cards). Pick the **home region** nearest you: it can't be changed, and
   the free computers are only made there.
2. **The computer:** Compute > Instances > Create instance. Image: Canonical Ubuntu (24.04). Shape:
   **VM.Standard.A1.Flex** at **1 OCPU, 6 GB** (marked "Always Free-eligible"; the free monthly amount covers one
   that size all month - don't raise it), or VM.Standard.E2.1.Micro (under "Specialty and previous generation", where
   offered; Brennan's region had none). "Out of capacity" (common for A1): another availability domain, or later. Networking: the defaults (a new network, a public
   IPv4 address). SSH keys: "Generate a key pair for me" - **download the private key** and keep it. Create, and note
   the **Public IP address** on the instance's page.
3. **Oracle's firewall:** on the instance's page, the subnet > its Security List > Add Ingress Rules: source
   `0.0.0.0/0`, IP protocol TCP, destination port `27015`.
4. **Put the server on it** (Windows PowerShell, in the folder with the files and the key):
   ```
   scp -r -i KEYFILE cloud-server ubuntu@PUBLIC-IP:~
   ssh -i KEYFILE ubuntu@PUBLIC-IP
   cd cloud-server
   sudo bash setup-server.sh
   ```
   On an ARM computer (A1) it first builds the server there (about a minute). It installs the server, opens port 27015 in the computer's own firewall (Oracle's Ubuntu lets in only SSH), starts
   it now and at every restart, and prints the invite code and the address to give friends.
5. **Everyone** (you too): `tandemJoin=PUBLIC-IP:27015`, `tandemInvite=CODE` (once), `tandemName=...`.
- **A new game version** needs the matching server: upload the new `cloud-server` folder (step 4's `scp`) and
  run `sudo bash setup-server.sh` in it again (keys, players and the code are kept). (Until the server updates itself: S3.)
- Who is playing: `sudo journalctl -u wallys-server -f` on the cloud computer.
- **Idle computers:** Oracle may stop a free computer that looks idle for 7 days (low CPU and network use - this
  server uses little of either). Start it again on the instance's page; an upgraded (pay-as-you-go) account isn't
  stopped (the free things stay free).

## A game that hosts (without the server)
The game can also host by itself (that player is player 1; no invite code):
Add to `WallysJourney.cfg` (next to the game):
```
tandemHost=27015
tandemName=Brennan
```
Start the game. The title says "Tandem: hosting on port 27015, waiting for friends".

## Friends (up to two)
```
tandemJoin=HOST-ADDRESS:27015
tandemName=YourName
```
HOST-ADDRESS is:
- **Same house / same Wi-Fi:** the host PC's local address (Windows: `ipconfig`, "IPv4 Address", e.g. 192.168.1.20).
- **Over the internet (free):** install **Tailscale** (tailscale.com, free) on every PC and sign in to the same
  Tailscale network (the host invites the friends). Use the host's Tailscale address (100.x.x.x, shown in the
  Tailscale app). No router changes needed.
Windows may ask the host whether WallysJourney.exe may use the network: allow it (Private networks).

To stop: delete those lines from `WallysJourney.cfg`. With none of them, the game never touches the network.
