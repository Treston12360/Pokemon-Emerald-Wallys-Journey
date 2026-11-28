#!/bin/bash
# Project Tandem: run the Wally's Journey server on an Ubuntu cloud computer (made for Oracle Cloud's free
# VM.Standard.E2.1.Micro; docs/TANDEM_HOWTO.md "A free cloud computer").
#
# Upload the whole cloud-server folder (this script, WallysJourneyServer-linux, src/), then in it:
#     sudo bash setup-server.sh
# Run it again after uploading a newer folder (each new game version needs the matching server): it swaps the server
# and restarts it. Players, keys and the invite code are kept.
# WallysJourneyServer-linux is for x86-64 computers (Oracle's VM.Standard.E2.1.Micro). On any other kind - Oracle's free
# ARM computer, VM.Standard.A1.Flex (Brennan 2026-11-27 18:53) - the server is built here from src/ (the same build of
# the game, its build id in src/tandem_build.h), with this computer's own compiler.
set -e
DIR=/opt/wallys-server
PORT=${PORT:-27015}
cd "$(dirname "$0")"
if [ "$(id -u)" != 0 ]; then echo "Run it with sudo:  sudo bash setup-server.sh"; exit 1; fi
if [ "$(uname -m)" != x86_64 ]; then
    if [ ! -f src/server/tandem_server.c ]; then echo "The src folder is missing: upload the whole cloud-server folder."; exit 1; fi
    echo "This computer is $(uname -m): building the server for it (about a minute)..."
    if ! command -v gcc >/dev/null; then
        apt-get update -qq && DEBIAN_FRONTEND=noninteractive apt-get install -y -qq gcc libc6-dev >/dev/null
    fi
    gcc -O2 -pthread -Isrc/server/linux_sdl -Isrc src/server/tandem_server.c src/tandem_net.c -o WallysJourneyServer-linux
    echo "Built."
fi
if [ ! -f WallysJourneyServer-linux ]; then echo "WallysJourneyServer-linux is missing: upload the whole cloud-server folder."; exit 1; fi

# a user of its own (not root), and its folder: the server, its settings, its players, its log
id wallys >/dev/null 2>&1 || useradd --system --home "$DIR" --shell /usr/sbin/nologin wallys
mkdir -p "$DIR"
systemctl stop wallys-server 2>/dev/null || true
install -m 755 WallysJourneyServer-linux "$DIR/"
[ -f "$DIR/WallysJourneyServer.cfg" ] || printf 'port=%s\n' "$PORT" > "$DIR/WallysJourneyServer.cfg"
chown -R wallys:wallys "$DIR"

# the computer's own firewall: Oracle's Ubuntu images let in only SSH - let in the game's port too, kept after restarts
if ! iptables -C INPUT -p tcp --dport "$PORT" -j ACCEPT 2>/dev/null; then
    iptables -I INPUT 1 -p tcp --dport "$PORT" -j ACCEPT
fi
if command -v netfilter-persistent >/dev/null; then netfilter-persistent save >/dev/null 2>&1 || true; fi

# started with the computer, restarted if it ever stops
cat > /etc/systemd/system/wallys-server.service <<EOF
[Unit]
Description=Wally's Journey server (Project Tandem)
After=network-online.target
Wants=network-online.target

[Service]
User=wallys
WorkingDirectory=$DIR
ExecStart=$DIR/WallysJourneyServer-linux
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF
systemctl daemon-reload
systemctl enable --now wallys-server >/dev/null
sleep 2
INVITE=$(grep '^invite=' "$DIR/WallysJourneyServer.cfg" | cut -d= -f2)
IP=$(curl -s --max-time 5 https://ifconfig.me 2>/dev/null || true)
echo
echo "=============================================================="
echo "  The Wally's Journey server is running."
echo "  Invite code:  $INVITE"
echo "  Everyone puts in WallysJourney.cfg:"
echo "      tandemJoin=${IP:-THIS-COMPUTERS-PUBLIC-IP}:$PORT"
echo "      tandemInvite=$INVITE"
echo "      tandemName=THEIR NAME"
echo "  Who is playing:   sudo journalctl -u wallys-server -f   (Ctrl+C to stop watching)"
echo "=============================================================="
