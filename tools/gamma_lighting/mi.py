import json,sys
def pv(p):
    v=p.get('Value')
    if isinstance(v,list): return {q.get('Name'):pv(q) for q in v if isinstance(q,dict)}
    return v
for f in sys.argv[1:]:
    d=json.load(open(f)); print('=====',f.split('/')[-1])
    imps=d['Imports']
    for e in d['Exports']:
        for p in e.get('Data') or []:
            n=p['Name']
            if n in('ScalarParameterValues','VectorParameterValues','TextureParameterValues','Parent'):
                if n=='Parent': i=p['Value']; print(' Parent:',imps[-i-1]['ObjectName'] if i<0 else i); continue
                for it in p['Value']:
                    x=pv(it); info=x.get('ParameterInfo') or {}; nm=info.get('Name') if isinstance(info,dict) else info
                    val=x.get('ParameterValue')
                    if n=='TextureParameterValues' and isinstance(val,int) and val<0: val=imps[-val-1]['ObjectName']
                    if isinstance(val,dict): val={k:round(v,4) if isinstance(v,float) else v for k,v in val.items()}
                    print(f'  {n[:6]} {nm} = {val}')
