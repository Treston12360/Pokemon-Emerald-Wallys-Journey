#!/usr/bin/env python3
"""usage: coplanar_check.py "DUMP.bin.*" ZMIN ZMAX   (the dump: WJPC_DUMP_TERRAIN=<map num> WJPC_DUMP_TERRAIN_FILE=<path>)

Session 014PwRDj, Brennan 2026-11-21 10:34 ("flag any coplanar ground/shadow draw passes"). Coplanar-overlap and tiny-step check over a terrain dump (WJPC_DUMP_TERRAIN): horizontal triangles rasterised on a
1/8-cell grid; a grid point covered by two triangles at the same height (z-fight on a GPU), or by two within 1/32 of
a tile in height (a near-coplanar pair a 24-bit depth buffer may not separate far away)."""
import struct,collections,glob,sys
pat, zmin, zmax = sys.argv[1], float(sys.argv[2]), float(sys.argv[3])
cov=collections.defaultdict(list)
def inside(px,pz,a):
    (x1,z1),(x2,z2),(x3,z3)=a
    d=(z2-z3)*(x1-x3)+(x3-x2)*(z1-z3)
    if abs(d)<1e-9: return False
    l1=((z2-z3)*(px-x3)+(x3-x2)*(pz-z3))/d; l2=((z3-z1)*(px-x3)+(x1-x3)*(pz-z3))/d; l3=1-l1-l2
    return l1>1e-4 and l2>1e-4 and l3>1e-4
for f in sorted(glob.glob(pat)):
  d=open(f,'rb').read(); n=len(d)//24
  for t in range(n//3):
    a=[struct.unpack_from('<6f',d,(3*t+j)*24) for j in range(3)]
    ys=[v[1] for v in a]
    if max(ys)-min(ys)>1e-4: continue
    xz=[(v[0],v[2]) for v in a]
    if max(p[1] for p in xz)<zmin or min(p[1] for p in xz)>zmax: continue
    gx=int(min(p[0] for p in xz)*8)
    while gx<=max(p[0] for p in xz)*8:
      gz=int(min(p[1] for p in xz)*8)
      while gz<=max(p[1] for p in xz)*8:
        if inside((gx+0.5)/8,(gz+0.5)/8,xz): cov[(gx,gz)].append((ys[0],f[-1],t,a[0][3],a[0][4]))
        gz+=1
      gx+=1
same=collections.Counter(); near=collections.Counter()
for k,v in cov.items():
    hs=sorted(x[0] for x in v)
    for i in range(1,len(hs)):
        if abs(hs[i]-hs[i-1])<1e-4: same[(k[0]//8,k[1]//8)]+=1
        elif hs[i]-hs[i-1]<1/32: near[(k[0]//8,k[1]//8)]+=1
print('cells with exactly coplanar overlap:',sorted(same))
print('cells with overlap closer than 1/32 tile:',sorted(near))
