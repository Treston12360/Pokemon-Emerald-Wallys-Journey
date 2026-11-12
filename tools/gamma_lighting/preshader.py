# Decode a cooked UE5.6 material's uniform preshader: which cb2 (Material uniform buffer) slot = which formula of which parameter.
import struct,sys,re,collections
d=open(sys.argv[1],'rb').read()
OPS=['Nop','ConstantZero','Constant','Parameter','Add','Sub','Mul','Div','Fmod','Modulo','Min','Max','Clamp','Sin','Cos','Tan','Asin','Acos','Atan','Atan2','Dot','Cross','Sqrt','Rcp','Length','Normalize','Saturate','Abs','Floor','Ceil','Round','Trunc','Sign','Frac','Fractional','Log2','Log10','ComponentSwizzle','AppendVector','TextureSize','TexelSize','ExtTexScaleRot','ExtTexOffset','RVTUniform','SVTUniform','Neg','PushValue','Less','Assign','Greater','LessEqual','GreaterEqual','Exp','Exp2','Log']
# frozen name table
def parse(i):
    out=[]
    while True:
        n=struct.unpack_from('<i',d,i)[0]
        if not (1<n<300): break
        s=d[i+4:i+4+n]
        if len(s)<n or s[-1]!=0 or not all(32<=c<127 for c in s[:-1]): break
        out.append(s[:-1].decode()); i+=4+n+4
    return out
st=d.find(b'FMaterialShaderMapContent')-4; names=parse(st)
# param info run (first one = SM5)
hits=[m.start()-12 for m in re.finditer(re.escape(bytes.fromhex('ffffffff02000000')),d)]
runs=[];cur=[]
for h in hits:
    if cur and h-cur[-1]==28: cur.append(h)
    else:
        if len(cur)>1: runs.append(cur)
        cur=[h]
if len(cur)>1: runs.append(cur)
run=max(runs[:2],key=len) if runs else []
if len(sys.argv)>2 and sys.argv[2].isdigit(): run=run[:int(sys.argv[2])]
pos=collections.defaultdict(list)
for i in range(len(d)-3): pos[struct.unpack_from('<I',d,i)[0]].append(i)
slots=run+[run[-1]+28]
import subprocess
_o=subprocess.run([sys.executable,'-I',__file__.replace('preshader.py','pn2.py'),sys.argv[1]],capture_output=True,text=True).stdout
_l=[l for l in _o.splitlines() if l.startswith('run len')][0]
import ast
pn=dict(enumerate(ast.literal_eval(_l[_l.index('['):])))
types={k:d[s+20] for k,s in enumerate(run)}
print('params:',{k:(pn.get(k,'?'),'vec' if types[k]==1 else 'scalar' if types[k]==0 else types[k]) for k in range(len(run))})
# opcode stream: try starts after run end
def dec(i,lim=4000):
    out=[];j=i
    while j<i+lim:
        op=d[j]
        if op==0: break
        if op>=len(OPS): return None,j
        nm=OPS[op]; j+=1
        if nm=='Parameter': out.append(f"P{struct.unpack_from('<H',d,j)[0]}"); j+=2
        elif nm=='Constant':
            t=d[j]; j+=1; n={1:1,2:2,3:3,4:4}.get(t)
            if n is None: return None,j
            out.append('C('+','.join(f'{x:g}' for x in struct.unpack_from(f'<{n}f',d,j))+')'); j+=4*n
        elif nm=='ComponentSwizzle':
            n=d[j]
            if not 1<=n<=4 or any(c>3 for c in d[j+1:j+1+n]): return None,j
            sw=''.join('xyzw'[c] for c in d[j+1:j+1+n]); out.append('.'+sw); j+=5
        elif nm in ('TextureSize','TexelSize'):
            out.append(f'{nm}(T{struct.unpack_from("<H",d,j)[0]})'); j+=11
        else: out.append(nm)
    return out,j
def valid(o):
    depth=0
    for t in o:
        if t.startswith(('P','C','Text')): depth+=1
        elif t.startswith('.') or t in('Rcp','Sqrt','Saturate','Abs','Neg','Frac','Floor','Ceil','Length','Normalize','Sin','Cos','Exp','Exp2','Log2','Log','Sign','Round','Trunc','Fractional','Log10','Tan'):
            if depth<1: return False
        else:
            if depth<2: return False
            depth-=1
    return True
best=(0,[],0)
for s_ in range(run[-1]+28, run[-1]+400):
    o,e=dec(s_)
    if o and valid(o) and o[0].startswith(('P','C')) and len(o)>best[0]: best=(len(o),o,s_)
n,o,s=best
print('stream@',s)
# split into fields: each field ends when stack depth returns to 1 after a full expression; simulate stack
stack=[];fields=[]
for t in o:
    if t.startswith('P') or t.startswith('C') or t.startswith('Text'): stack.append(t if not t.startswith('P') else pn.get(int(t[1:]),t))
    elif t.startswith('.'): stack[-1]=f"{stack[-1]}{t}"
    elif t in('Rcp','Sqrt','Saturate','Abs','Neg','Frac','Floor','Ceil','Length','Normalize','Sin','Cos','Exp','Exp2','Log2','Log','Sign','Round','Trunc','Fractional','Log10','Tan'):
        stack[-1]=f"{t}({stack[-1]})"
    elif t in('Add','Sub','Mul','Div','Min','Max','Fmod','Modulo','Dot','Cross','AppendVector','Atan2','Less','Greater'):
        b=stack.pop(); a=stack.pop(); sym={'Add':'+','Sub':'-','Mul':'*','Div':'/'}.get(t)
        stack.append(f"({a} {sym} {b})" if sym else f"{t}({a}, {b})")
    else: stack.append(t)
print('values pushed (in order -> packed into the Material buffer):')
for k,v in enumerate(stack): print(f'  #{k}: {v}')
# defaults: table right after the opcode stream's terminating zero
o2,e=dec(s)
tab=e
print('default values (parent material):')
for k,sl in enumerate(run):
    t=d[sl+20]; off=struct.unpack_from('<I',d,sl+24)[0]
    if '-v' in sys.argv: print('   raw',k,pn.get(k),d[sl:sl+28].hex(' '))
    if t==0 and off<4096: print(f'  {pn.get(k,"?")}: {struct.unpack_from("<f",d,tab+off)[0]:g}')
    elif t==1 and off<4096: print(f'  {pn.get(k,"?")}: '+', '.join(f'{x:g}' for x in struct.unpack_from("<4f",d,tab+off)))
