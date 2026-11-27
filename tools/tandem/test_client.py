# a minimal Tandem client for tests: python3 proto_client.py PORT BUILD NAME [seconds]
import socket, struct, sys, time
port, build, name = int(sys.argv[1]), sys.argv[2], sys.argv[3]
secs = float(sys.argv[4]) if len(sys.argv) > 4 else 4
s = socket.create_connection(('127.0.0.1', port), timeout=5)
def send(t, p): s.sendall(struct.pack('<HB', len(p) + 1, t) + p)
send(1, struct.pack('<H', 1) + build.encode().ljust(40, b'\0')[:40] + name.encode().ljust(16, b'\0')[:16])
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
        if t == 2: txt = 'slot %d host %s build %s' % (p[0], p[42:58].split(b'\0')[0].decode(), p[2:42].split(b'\0')[0].decode())
        if t == 4: txt = 'slot %d %s %s' % (p[0], 'joined' if p[1] else 'left', p[2:18].split(b'\0')[0].decode())
        print('got', names.get(t, t), txt)
        if t == 6: send(7, p)
send(8, b''); s.close()
