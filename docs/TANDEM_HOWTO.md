# Project Tandem - how to connect (step 1: connecting only)

Right now connecting only checks the games can talk: you see who is connected in the window's title, and
`WallysJourney_tandem.log` (next to the game) lists who joined, the hello messages and the delay. You don't see each
other in the game yet - that is the next step.

Everyone needs the **same game zip** (the game refuses a different build and says so in the log).

## The host (one player)
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
