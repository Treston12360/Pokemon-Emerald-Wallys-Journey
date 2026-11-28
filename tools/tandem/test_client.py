# a minimal Tandem client for tests: python3 test_client.py PORT BUILD NAME [seconds] [invite] [key]
# (protocol 6: the hello carries an invite code and a key - WallysJourneyServer.exe asks for one of them)
import socket, struct, sys, time
port, build, name = int(sys.argv[1]), sys.argv[2], sys.argv[3]
secs = float(sys.argv[4]) if len(sys.argv) > 4 else 4
invite = sys.argv[5] if len(sys.argv) > 5 else ''
key = sys.argv[6] if len(sys.argv) > 6 else ''
PROTOCOL = 6
s = socket.create_connection(('127.0.0.1', port), timeout=5)
def send(t, p): s.sendall(struct.pack('<HB', len(p) + 1, t) + p)
send(1, struct.pack('<H', PROTOCOL) + build.encode().ljust(40, b'\0')[:40] + name.encode().ljust(16, b'\0')[:16]
        + invite.encode().ljust(16, b'\0')[:16] + key.encode().ljust(40, b'\0')[:40])
buf = b''; end = time.time() + secs
names = {1:'HELLO',2:'WELCOME',3:'REFUSE',4:'PEER',5:'TEXT',6:'PING',7:'PONG',8:'BYE'}
s.settimeout(0.5)
while time.time() < end:
    try:
        d = s.recv(4096)
        if not d: print('closed by host'); break
        buf += d
    except socket.timeout:
        continue
    while len(buf) >= 2:
        n = struct.unpack('<H', buf[:2])[0]
        if len(buf) < 2 + n: break
        t, p = buf[2], buf[3:2 + n]; buf = buf[2 + n:]
        txt = p.split(b'\0')[0].decode(errors='replace') if t in (3, 5) else ''
        if t == 2: txt = 'slot %d host %s build %s key %s' % (p[0], p[42:58].split(b'\0')[0].decode(), p[2:42].split(b'\0')[0].decode(), p[59:99].split(b'\0')[0].decode())
        if t == 4: txt = 'slot %d %s %s' % (p[0], 'joined' if p[1] else 'left', p[2:18].split(b'\0')[0].decode())
        print('got', names.get(t, t), txt)
        if t == 6: send(7, p)
send(8, b''); s.close()
