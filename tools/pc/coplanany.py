# coplanany.py DIR x0 x1 z0 z1 : overlapping coplanar faces (any orientation) among terrain pages, models, trees, statics
import struct,sys,collections,glob
D,x0,x1,z0,z1=sys.argv[1],*map(float,sys.argv[2:6])
TOL=float(sys.argv[6]) if len(sys.argv)>6 else 1e-4
T=[]
def addtri(src,a,b,c):
    if all(x0<=v[0]<=x1 and z0<=v[2]<=z1 for v in (a,b,c)): T.append((src,a,b,c))
for f in glob.glob(D+'/t.[0-9]')+[D+'/t.models',D+'/t.trees']:
    d=open(f,'rb').read(); V=[struct.unpack_from('<6f',d,24*i)[:3] for i in range(len(d)//24)]
    for t in range(len(V)//3): addtri(f.split('/')[-1],*V[3*t:3*t+3])
cur=None;buf=[];off=(0,0,0)
for line in open(D+'/t.statics'):
    w=line.split()
    if w[0]=='static':
        cur=w[1]; off=tuple(map(float,w[2:5])); buf=[]
    elif w[0]=='v':
        buf.append(tuple(float(w[i])+off[i-1] for i in (1,2,3)))
        if len(buf)==3: addtri('static'+cur,*buf); buf=[]
def plane(a,b,c):
    u=[b[i]-a[i] for i in range(3)]; v=[c[i]-a[i] for i in range(3)]
    n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
    L=sum(x*x for x in n)**.5
    if L<1e-9: return None
    n=[x/L for x in n]
    k=max(range(3),key=lambda i:abs(n[i]))
    if n[k]<0: n=[-x for x in n]
    return n,sum(n[i]*a[i] for i in range(3))
def inside(p,t,ax):
    (a,b,c)=[[v[i] for i in range(3) if i!=ax] for v in t]
    def s(p1,p2,p3): return (p1[0]-p3[0])*(p2[1]-p3[1])-(p2[0]-p3[0])*(p1[1]-p3[1])
    d1,d2,d3=s(p,a,b),s(p,b,c),s(p,c,a)
    neg=(d1< -1e-6) or (d2< -1e-6) or (d3< -1e-6); pos=(d1>1e-6) or (d2>1e-6) or (d3>1e-6)
    return not(neg and pos) and (abs(d1)>1e-6 and abs(d2)>1e-6 and abs(d3)>1e-6)
G=collections.defaultdict(list)
for t in T:
    p=plane(*t[1:])
    if p: G[(round(p[0][0],2),round(p[0][1],2),round(p[0][2],2))].append((p[1],t))
hits=collections.Counter()
for k,ts in G.items():
    if len(ts)<2: continue
    ax=max(range(3),key=lambda i:abs(k[i]))
    for i in range(len(ts)):
        for j in range(i+1,len(ts)):
            (dA,A),(dB,B)=ts[i],ts[j]
            if abs(dA-dB)>TOL: continue
            if A[0]==B[0] and A[0].startswith('t.'): continue
            cA=[sum(v[q] for v in A[1:])/3 for q in range(3)]
            cB=[sum(v[q] for v in B[1:])/3 for q in range(3)]
            for c,other in ((cA,B),(cB,A)):
                pt=[c[q] for q in range(3) if q!=ax]
                if inside(pt,other[1:],ax):
                    hits[(A[0],B[0],k,round(dA,4),round(dB-dA,4), tuple(round(x,2) for x in cA))]+=1; break
for h,n in sorted(hits.items())[:60]: print(h)
print(len(hits),'overlapping pairs')
