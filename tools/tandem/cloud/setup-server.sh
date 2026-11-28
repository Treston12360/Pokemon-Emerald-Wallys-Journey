#!/bin/bash
# Project Tandem: run the Wally's Journey server on an Ubuntu cloud computer (made for Oracle Cloud's free
# VM.Standard.E2.1.Micro; docs/TANDEM_HOWTO.md "A free cloud computer").
#
# Put WallysJourneyServer-linux next to this script, then:   sudo bash setup-server.sh
# Run it again after uploading a newer WallysJourneyServer-linux (each new game version needs the matching server):
# it swaps the file and restarts the server. Players, keys and the invite code are kept.
set -e
DIR=/opt/wallys-server
PORT=${PORT:-27015}
cd "$(dirname "$0")"
if [ ! -f WallysJourneyServer-linux ]; then echo "Put WallysJourneyServer-linux next to this script first."; exit 1; fi
if [ "$(id -u)" != 0 ]; then echo "Run it with sudo:  sudo bash setup-server.sh"; exit 1; fi

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
