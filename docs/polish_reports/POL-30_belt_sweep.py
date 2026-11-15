import json,os,re,sys
R="/home/claude/wj-3d"
ids={}
for l in open(R+"/include/constants/map_groups.h"):
    m=re.match(r"\s*(MAP_\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)",l)
    if m: ids[m.group(1)]=(int(m.group(3)),int(m.group(2)))
lay={l["id"]:l for l in json.load(open(R+"/data/layouts/layouts.json"))["layouts"] if "id" in l}
maps={}
for d in os.listdir(R+"/data/maps"):
    p=f"{R}/data/maps/{d}/map.json"
    if os.path.exists(p):
        j=json.load(open(p)); maps[j["id"]]=j
pairs=[];seen=set()
for a,j in maps.items():
    if "Frlg" in j["name"] or a not in ids: continue
    if j.get("map_type") not in ("MAP_TYPE_TOWN","MAP_TYPE_CITY","MAP_TYPE_ROUTE","MAP_TYPE_OCEAN_ROUTE"): continue
    la=lay[j["layout"]]; aw,ah=la["width"],la["height"]
    for c in j.get("connections") or []:
        b=c["map"]; d=c["direction"]; o=c["offset"]
        if b not in maps or d not in("left","right","up","down"): continue
        lb=lay[maps[b]["layout"]]; bw,bh=lb["width"],lb["height"]
        key=tuple(sorted((a,b)))
        if key in seen: continue
        seen.add(key)
        if d in("left","right"):
            lo,hi=max(0,o),min(ah,o+bh); y=(lo+hi)//2
            bx0=-bw if d=="left" else aw; by0=o
            pa=(2 if d=="left" else aw-3, y); pb=(bw-3 if d=="left" else 2, y-o)
        else:
            lo,hi=max(0,o),min(aw,o+bw); x=(lo+hi)//2
            by0=-bh if d=="up" else ah; bx0=o
            pa=(x, 2 if d=="up" else ah-3); pb=(x-o, bh-3 if d=="up" else 2)
        pairs.append((a,b,pa,pb,(bx0,by0)))
if sys.argv[1]=="script":
    for a,b,pa,pb,_ in pairs:
        for m,p in((a,pa),(b,pb)):
            print(f"warp {ids[m][0]} {ids[m][1]} {p[0]} {p[1]}"); print("wait 240")
    print("quit")
else:
    tabs=[];cur=None
    for l in open(sys.argv[2]):
        t=l.split()
        if t[0]=="MAP": cur={}; tabs.append(((int(t[1]),int(t[2])),(int(t[3]),int(t[4])),cur))
        else: cur[(int(t[0]),int(t[1]))]=(int(t[2]),int(t[3]))
    def find(m,p):
        for k,pp,t in tabs:
            if k==ids[m] and abs(pp[0]-p[0])<=1 and abs(pp[1]-p[1])<=1: return t
    ta=tm=th=0;bad=[]
    for a,b,pa,pb,(bx0,by0) in pairs:
        A=find(a,pa);B=find(b,pb)
        if A is None or B is None: print("MISSING",a,b); continue
        n=dm=dh=0
        for (x,y),(mt,h) in A.items():
            if abs(x-pa[0])>24 or abs(y-pa[1])>24: continue
            q=B.get((x-bx0,y-by0))
            if q is None: continue
            n+=1; dm+=q[0]!=mt; dh+=abs(q[1]-h)>1
        ta+=n;tm+=dm;th+=dh
        if dm or dh: bad.append((dm+dh,a,b,n,dm,dh))
    print(len(pairs),"links;",ta,"belt cells compared;",tm,"differ in art;",th,"differ in height")
    for r in sorted(bad,reverse=True): print(r)
