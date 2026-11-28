# Project Tandem - how to connect (steps 1-4 and the server: connecting, seeing each other, across maps, player cards)

You see who is connected in the window's title, and `WallysJourney_tandem.log` (next to the game) lists who joined,
the hello messages and the delay. When a friend is on the same map as you, or on the map just across a border
(the next route or town), they show up as a person walking around (walking, running, biking or surfing as they do),
with their following Pokemon; you walk through each other. They disappear when they go through a door or fly
somewhere you can't see, and come back when they come out. Everyone shows in their own outfit.

Face a friend and press A for the menu: Trade, Player Card, Battle, Team Up. Player Card shows their card (their outfit,
money, Pokedex, time, badges). Trade, battles and teaming up are later steps.

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
  - A free always-on cloud computer (e.g. Oracle Cloud Always Free) can run the server instead of your PC - then it
    is up even when your PC is off (needs a Linux build of the server: ask).

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
