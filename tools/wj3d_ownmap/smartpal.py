import numpy as np, sys, os
from PIL import Image
def kmeans(points,k,iters=30,seed=1):
    rng=np.random.default_rng(seed); uniq=np.unique(points,axis=0)
    if len(uniq)<=k: return uniq.astype(np.float32)
    cent=[uniq[rng.integers(len(uniq))]]
    for _ in range(k-1):
        d=np.min(((uniq[:,None]-np.array(cent)[None])**2).sum(-1),1); cent.append(uniq[rng.choice(len(uniq),p=d/d.sum())])
    cent=np.array(cent,np.float32)
    for _ in range(iters):
        l=((points[:,None]-cent[None])**2).sum(-1).argmin(1)
        for j in range(k):
            m=points[l==j]
            if len(m): cent[j]=m.mean(0)
    return cent
def convert(rgb, labs, npal, smart, rows=14, slots=(2,3,4,6)):
    img=np.round(rgb[:rows*8].astype(np.float32)/255*31)
    tiles=[img[ty*8:(ty+1)*8,tx*8:(tx+1)*8].reshape(-1,3) for ty in range(rows) for tx in range(30)]
    tl=[labs[ty*8:(ty+1)*8,tx*8:(tx+1)*8].reshape(-1) for ty in range(rows) for tx in range(30)]
    if smart:
        # first split by what the block mostly shows: trees / tall grass / open ground; extra sets go to the busiest group
        dom=np.array([np.bincount(l,minlength=3).argmax() for l in tl])
        groups=[g for g in (1,2,0) if (dom==g).any()]
        alloc={g:1 for g in groups}
        while sum(alloc.values())<npal:
            # give an extra set to the group with the most colour variety
            var={g:np.concatenate([tiles[i] for i in range(len(tiles)) if dom[i]==g]).std(0).sum()/alloc[g] for g in groups}
            alloc[max(var,key=var.get)]+=1
        lab=np.zeros(len(tiles),int); base=0
        for g in groups:
            idx=[i for i in range(len(tiles)) if dom[i]==g]
            feat=np.array([tiles[i].mean(0) for i in idx])
            sub=((feat[:,None]-kmeans(feat,alloc[g])[None])**2).sum(-1).argmin(1) if alloc[g]>1 else np.zeros(len(idx),int)
            for k,i in enumerate(idx): lab[i]=base+sub[k]
            base+=alloc[g]
    else:
        means=np.array([t.mean(0) for t in tiles]); lab=((means[:,None]-kmeans(means,npal)[None])**2).sum(-1).argmin(1)
    for _ in range(5):
        pals=[]
        for g in range(npal):
            px=np.concatenate([tiles[i] for i in range(len(tiles)) if lab[i]==g] or [np.zeros((1,3))])
            pals.append(np.clip(np.round(kmeans(px,15)),0,31))
        err=np.array([[((t[:,None]-p[None])**2).sum(-1).min(1).sum() for p in pals] for t in tiles]); lab=err.argmin(1)
    out=np.zeros((160,240,3),np.uint8)
    for i,t in enumerate(tiles):
        ty,tx=divmod(i,30); p=pals[lab[i]]
        idx=((t[:,None]-p[None])**2).sum(-1).argmin(1)
        out[ty*8:(ty+1)*8,tx*8:(tx+1)*8]=(p[idx]*255/31).reshape(8,8,3).astype(np.uint8)
    return out, pals, lab, tiles
def write(outdir, pals, lab, tiles, slots=(2,3,4,6)):
    os.makedirs(outdir,exist_ok=True)
    uniq={bytes(64):0}; tl=[np.zeros(64,np.uint8)]; tmap=np.zeros((32,32),np.uint16)
    for i,t in enumerate(tiles):
        p=pals[lab[i]]; idx=(((t[:,None]-p[None])**2).sum(-1).argmin(1)+1).astype(np.uint8)
        k=idx.tobytes()
        if k not in uniq: uniq[k]=len(tl); tl.append(idx)
        ty,tx=divmod(i,30); tmap[ty,tx]=uniq[k]|(slots[lab[i]]<<12)
    for ty in range(14,32):
        for tx in range(32): tmap[ty,tx]=slots[0]<<12
    n=len(tl); w=16; sheet=np.zeros((((n+w-1)//w)*8,w*8),np.uint8)
    for i,t in enumerate(tl): r,c=divmod(i,w); sheet[r*8:(r+1)*8,c*8:(c+1)*8]=t.reshape(8,8)
    im=Image.fromarray(sheet,'P'); flat=[0,0,0]+[int(v)*255//31 for v in pals[0].reshape(-1)]; im.putpalette(flat+[0]*(768-len(flat))); im.save(outdir+'/tiles.png')
    open(outdir+'/tilemap.bin','wb').write(tmap.astype('<u2').tobytes())
    with open(outdir+'/palette.pal','w') as f:
        f.write('JASC-PAL\r\n0100\r\n%d\r\n'%(16*len(pals)))
        for p in pals:
            p=np.pad(p,((0,15-len(p)),(0,0)))
            f.write('0 0 0\r\n')
            for c in p: f.write('%d %d %d\r\n'%tuple(int(v)*255//31 for v in c))
    return n
