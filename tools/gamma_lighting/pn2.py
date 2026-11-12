import struct,sys
d=open(sys.argv[1],'rb').read()
def parse(i):
    out=[]
    while True:
        n=struct.unpack_from('<i',d,i)[0]
        if not (1<n<300): break
        s=d[i+4:i+4+n]
        if len(s)<n or s[-1]!=0: break
        out.append(s[:-1].decode('latin1')); i+=4+n+4
    return out
names=parse(d.find(b'FMaterialShaderMapContent')-4)
lo=names.index('FRHIUniformBufferResourceInitializer')+1; hi=next(i for i,n in enumerate(names) if n.startswith('PCD3D'))
# scan for patch-table chains: idx,num(0),cnt,offsets...
def chain(p):
    out=[]
    while p+12<=len(d):
        idx,num,cnt=struct.unpack_from('<3I',d,p)
        if not (idx<len(names) and num==0 and 1<=cnt<=400): break
        if p+12+4*cnt>len(d): break
        offs=struct.unpack_from(f'<{cnt}I',d,p+12); out.append((idx,offs)); p+=12+4*cnt
    return out,p
best=None
for p in range(0,len(d)-12):
    c,e=chain(p)
    if len(c)>=len(names)//2 and (best is None or len(c)>len(best[0])): best=(c,p); 
    if best and p>best[1]+40000: break
c,p=best
pm={}
for idx,offs in c:
    if lo<=idx<hi:
        for o in offs: pm[o]=names[idx]
ks=sorted(pm)
print('patch table @',p,'param name slots:',len(ks))
# group with stride 28 runs
runs=[];cur=[ks[0]]
for k in ks[1:]:
    if k-cur[-1]==28: cur.append(k)
    else: runs.append(cur); cur=[k]
runs.append(cur)
for r in runs: print('run len',len(r),[pm[k] for k in r])
