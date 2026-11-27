/*
 * Project Tandem (Brennan's online multiplayer, docs/TANDEM_TASKS.md, docs/TANDEM_AUDIT.md) - T1: the network link.
 *
 * Two or three PCs running the same build connect: one hosts, the others join (Brennan 2026-11-27: "3 first";
 * connecting with "a free tool" - a LAN address, or a free private network such as Tailscale). On joining, each side
 * checks the other runs the same build (git commit) and refuses a different one. Connected, they swap small test
 * messages (hello, pings). Nothing in the game uses it yet (T2: seeing each other walk).
 *
 * Rules (TANDEM_TASKS.md): the network runs on its own thread and never makes the game wait; nothing here touches the
 * game's data or drawing; with nothing set, nothing starts - single-player exactly as before.
 *
 * Settings, in WallysJourney.cfg (or the same names as environment variables, upper case with WJPC_ in front, for
 * tests - WJPC_TANDEM_HOST etc.):
 *   tandemHost=PORT          host on PORT (e.g. tandemHost=27015)
 *   tandemJoin=ADDRESS:PORT  join the host at ADDRESS (a name or an IP, e.g. 100.64.1.2:27015)
 *   tandemName=NAME          the name friends see (up to 15 letters; default "Player")
 * What happens is written to WallysJourney_tandem.log next to the settings file, and shown in the window's title.
 */
#ifndef GUARD_TANDEM_NET_H
#define GUARD_TANDEM_NET_H

#include <stdbool.h>
#include <stdio.h>

#define TANDEM_MAX_PLAYERS 3
#define TANDEM_DEFAULT_PORT 27015

/* a line of the settings file: TRUE if it was one of Tandem's (and taken) */
bool TandemNet_ConfigLine(const char *line);
/* Tandem's lines for the settings file (only those that are set) */
void TandemNet_WriteConfig(FILE *configFile);
/* start hosting or joining if the settings ask for it (logPath: the log file); does nothing otherwise */
void TandemNet_Start(const char *logPath);
/* close every connection and stop the thread */
void TandemNet_Stop(void);
/* TRUE while hosting or joining (connected or trying) */
bool TandemNet_Active(void);
/* a short line for the window title ("" when not active); changes - compare *gen with the last one seen */
const char *TandemNet_StatusText(unsigned *gen);

#endif
