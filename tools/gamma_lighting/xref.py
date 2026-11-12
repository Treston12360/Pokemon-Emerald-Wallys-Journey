import sys,struct,numpy as np
from capstone import *
E='/mnt/user-data/uploads/gamma-emerald-ea-windows/PokemonEmerald/Binaries/Win64/PokemonEmerald-Win64-Shipping.exe'
d=open(E,'rb').read()
TEXT_RVA,TEXT_OFF,TEXT_SZ=0x1000,0x600,0x7873a3c
text=np.frombuffer(d[TEXT_OFF:TEXT_OFF+TEXT_SZ],np.uint8)
def xrefs(target_rva):
    # find rip-relative disp32 such that rva_of_next_instr + disp = target. Scan all 4-byte positions.
    b=d[TEXT_OFF:TEXT_OFF+TEXT_SZ]
    arr=np.frombuffer(b[:len(b)//4*4+0],np.uint8)
    out=[]
    for shift in range(4):
        v=np.frombuffer(b[shift:shift+((len(b)-shift)//4)*4],dtype='<i4')
        pos=np.arange(len(v))*4+shift
        nxt=TEXT_RVA+pos+4
        hit=np.nonzero(nxt+v==target_rva)[0]
        out+= [int(pos[h]) for h in hit]
    return sorted(out)
def dis(off_in_text,before=60,after=80):
    md=Cs(CS_ARCH_X86,CS_MODE_64)
    start=max(0,off_in_text-before)
    code=d[TEXT_OFF+start:TEXT_OFF+off_in_text+after]
    # resync: try starts until an instruction lands on the disp-containing insn
    for k in range(0,before):
        ins=list(md.disasm(code[k:],0x140000000+TEXT_RVA+start+k))
        addrs=[i.address for i in ins]
        tgt=0x140000000+TEXT_RVA+off_in_text
        if any(i.address<tgt<i.address+i.size for i in ins):
            return '\n'.join(f'{i.address:x}: {i.mnemonic} {i.op_str}' for i in ins)
    return '?'
if __name__=='__main__':
    s=sys.argv[1].encode('utf-16-le')
    i=d.find(s+b'\0\0'); 
    import pefile; pe=pefile.PE(E,fast_load=True); rva=pe.get_rva_from_offset(i)
    xs=xrefs(rva); print('string rva',hex(rva),'xrefs',len(xs))
    for x in xs[:3]: print('-----'); print(dis(x,int(sys.argv[2]) if len(sys.argv)>2 else 60,int(sys.argv[3]) if len(sys.argv)>3 else 80))
