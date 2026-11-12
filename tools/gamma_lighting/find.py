import pickle,sys,glob,os
a=pickle.load(open('/home/claude/ue/sh/arch.pkl','rb')); M={h:i for i,h in enumerate(a['maps'])}
for f in sys.argv[1:]:
    d=open(f,'rb').read(); hits=[]
    for i in range(len(d)-20):
        j=M.get(d[i:i+20])
        if j is not None: hits.append((i,j))
    print(os.path.basename(f), [(i,j,a['me'][j][1]) for i,j in hits])
