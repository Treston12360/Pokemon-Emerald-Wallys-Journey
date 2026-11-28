/*
 * Project Tandem: the server program, WallysJourneyServer.exe (docs/TANDEM_HOWTO.md).
 *
 * Brennan 2026-11-27: "click a .exe and launch the server" - a window that keeps friends connected to each other; the
 * game itself plays alone whenever it can't reach it, and joins again by itself. The network side is the game's own
 * (src/platform/tandem_net.c), started with no player of its own.
 *
 * Next to it: WallysJourneyServer.cfg (port=, invite=, name=; an invite code is made on the first start),
 * WallysJourneyServer_players.txt (the players let in so far: key and name), WallysJourneyServer.log.
 * Plain C, built without the game's text preprocessor (Makefile.pc).
 */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "../tandem_net.h"

#define CFG_FILE "WallysJourneyServer.cfg"

static unsigned sPort = TANDEM_DEFAULT_PORT;
static char sInvite[16];
static char sName[16] = "the server";

static void MakeInvite(void)
{
    /* (no 0/O or 1/I: read out loud or typed, they get mixed up) */
    static const char letters[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    unsigned x = (unsigned)time(NULL) ^ (unsigned)SDL_GetPerformanceCounter();
    int i;

    for (i = 0; i < 6; i++)
    {
        x = x * 1103515245u + 12345u;
        sInvite[i] = letters[(x >> 16) % (sizeof(letters) - 1)];
    }
    sInvite[6] = '\0';
}

static void ReadSettings(void)
{
    FILE *f = fopen(CFG_FILE, "r");
    char line[256];

    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f) != NULL)
    {
        char *end = line + strlen(line);
        unsigned v;

        while (end > line && (end[-1] == '\n' || end[-1] == '\r' || end[-1] == ' '))
            *--end = '\0';
        if (sscanf(line, "port=%u", &v) == 1 && v > 0 && v < 65536)
            sPort = v;
        else if (strncmp(line, "invite=", 7) == 0)
            snprintf(sInvite, sizeof(sInvite), "%s", line + 7);
        else if (strncmp(line, "name=", 5) == 0 && line[5] != '\0')
            snprintf(sName, sizeof(sName), "%s", line + 5);
    }
    fclose(f);
}

static void WriteSettings(void)
{
    FILE *f = fopen(CFG_FILE, "w");

    if (f == NULL)
        return;
    fprintf(f, "# Wally's Journey server settings (Project Tandem). Change them with the server closed.\n");
    fprintf(f, "# port: the port friends connect to. invite: the code new players type once.\n");
    fprintf(f, "# name: shown to players (up to 15 letters).\n");
    fprintf(f, "port=%u\ninvite=%s\nname=%s\n", sPort, sInvite, sName);
    fclose(f);
}

/* this PC's addresses on the home network, for friends in the same house */
static void PrintLocalAddresses(void)
{
#ifdef _WIN32
    char host[256];
    struct addrinfo hints, *res = NULL, *a;

    if (gethostname(host, sizeof(host)) != 0)
        return;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (getaddrinfo(host, NULL, &hints, &res) != 0)
        return;
    for (a = res; a != NULL; a = a->ai_next)
    {
        char ip[64];
        inet_ntop(AF_INET, &((struct sockaddr_in *)a->ai_addr)->sin_addr, ip, sizeof(ip));
        printf("      %s:%u\n", ip, sPort);
    }
    freeaddrinfo(res);
#else
    struct ifaddrs *list, *a;

    if (getifaddrs(&list) != 0)
        return;
    for (a = list; a != NULL; a = a->ifa_next)
        if (a->ifa_addr != NULL && a->ifa_addr->sa_family == AF_INET)
        {
            char ip[64];
            inet_ntop(AF_INET, &((struct sockaddr_in *)a->ifa_addr)->sin_addr, ip, sizeof(ip));
            if (strcmp(ip, "127.0.0.1") != 0)
                printf("      %s:%u\n", ip, sPort);
        }
    freeifaddrs(list);
#endif
}

int main(int argc, char **argv)
{
    unsigned lastGen = 0;

    (void)argc;
    (void)argv;
    SDL_SetMainReady();
    SDL_Init(0);
    ReadSettings();
    if (sInvite[0] == '\0')
        MakeInvite();
    WriteSettings();

    printf("==============================================================\n");
    printf("  Wally's Journey - Project Tandem server\n");
    printf("==============================================================\n");
    printf("  Leave this window open while friends play. Close it to stop.\n\n");
    printf("  Invite code for new players:  %s\n", sInvite);
    printf("  (they type it once; after that their PC is remembered)\n\n");
    printf("  Friends put this in WallysJourney.cfg (next to the game):\n");
    printf("      tandemJoin=ADDRESS:%u\n", sPort);
    printf("      tandemInvite=%s\n", sInvite);
    printf("      tandemName=THEIR NAME\n");
    printf("  ADDRESS: 127.0.0.1 on this PC; in the same house, this PC's address:\n");
#ifdef _WIN32
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        PrintLocalAddresses();
        WSACleanup();
    }
#else
    PrintLocalAddresses();
#endif
    printf("  Over the internet: see docs (TANDEM_HOWTO) - your public address,\n");
    printf("  a playit.gg address, or a Tailscale address.\n");
    printf("==============================================================\n\n");
    fflush(stdout);

    if (!TandemNet_StartServer(sPort, sInvite, sName, "WallysJourneyServer_players.txt", "WallysJourneyServer.log"))
    {
        printf("The server couldn't start. Press Enter to close.\n");
        getchar();
        return 1;
    }
    /* (the network thread does the work; the window shows who is playing when that changes) */
    for (;;)
    {
        unsigned gen;
        const char *status;

        SDL_Delay(250);
        if (!TandemNet_Active())
            break;
        status = TandemNet_StatusText(&gen);
        if (gen != lastGen)
        {
            lastGen = gen;
            printf("   >> %s\n", status);
            fflush(stdout);
        }
    }
    TandemNet_Stop();
    return 0;
}
