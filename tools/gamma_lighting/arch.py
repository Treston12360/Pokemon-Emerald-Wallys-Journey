import struct,sys,pickle
A=sys.argv[1]; d=open(A,'rb').read(); o=0
def u32():
    global o; v=struct.unpack_from('<I',d,o)[0]; o+=4; return v
ver=u32()
n=u32(); maps=[d[o+i*20:o+i*20+20] for i in range(n)]; o+=20*n
n=u32(); shs=[d[o+i*20:o+i*20+20] for i in range(n)]; o+=20*n
n=u32(); me=[struct.unpack_from('<4I',d,o+i*16) for i in range(n)]; o+=16*n
n=u32(); ents=[]
for i in range(n):
    off,sz,usz,fr=struct.unpack_from('<QIIB',d,o); o+=17; ents.append((off,sz,usz,fr))
n=u32(); pre=[struct.unpack_from('<QQ',d,o+i*16) for i in range(n)]; o+=16*n
n=u32(); idx=list(struct.unpack_from(f'<{n}I',d,o)); o+=4*n
print('ver',ver,'maps',len(maps),'shaders',len(shs),'entries',len(ents),'codestart',o, 'filesize',len(d))
print('first entries',ents[:3])
pickle.dump(dict(maps=maps,shs=shs,me=me,ents=ents,idx=idx,code=o),open(sys.argv[2],'wb'))
