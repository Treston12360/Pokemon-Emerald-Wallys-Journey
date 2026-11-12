import pickle,sys,ooz,os,subprocess
a=pickle.load(open('/home/claude/ue/sh/arch.pkl','rb'))
A=sys.argv[1]; mi=int(sys.argv[2]); out=sys.argv[3]; os.makedirs(out,exist_ok=True)
f=open(A,'rb'); so,ns,_,_=a['me'][mi]
for k in range(ns):
    si=a['idx'][so+k]; off,sz,usz,fr=a['ents'][si]
    f.seek(a['code']+off); c=f.read(sz)
    raw=c if sz==usz else ooz.decompress(c,usz)
    p=raw.find(b'DXBC')
    dx=raw[p:p+int.from_bytes(raw[p+24:p+28],'little')] if p>=0 else b''
    fn=f"{out}/s{k:02d}_f{fr}.dxbc"; open(fn,'wb').write(dx)
    asm=subprocess.run(['vkd3d-compiler','-x','dxbc-tpf','-b','d3d-asm',fn],capture_output=True,text=True)
    open(fn[:-5]+'.asm','w').write(asm.stdout+asm.stderr)
    print(k,'freq',fr,'dxbc@',p,'len',len(dx),'asm lines',asm.stdout.count('\n'), raw[:8].hex())
