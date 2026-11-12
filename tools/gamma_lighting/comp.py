import json,sys
d=json.load(open(sys.argv[1])); imps=d['Imports']; ex=d['Exports']
pat=sys.argv[2].split(',')
def nm(i): return imps[-i-1]['ObjectName'] if i<0 else (ex[i-1]['ObjectName'] if i>0 else None)
def pv(p):
    v=p.get('Value')
    if isinstance(v,list): return {q.get('Name'):pv(q) for q in v if isinstance(q,dict)} if v and isinstance(v[0],dict) and 'Name' in v[0] else [pv(q) if isinstance(q,dict) else q for q in v]
    if isinstance(v,dict): return {k:(round(x,4) if isinstance(x,float) else x) for k,x in v.items() if k!='$type'}
    if 'ObjectProperty' in p.get('$type','') and isinstance(v,int): return nm(v)
    return v
for i,e in enumerate(ex,1):
    c=nm(e.get('ClassIndex',0)) or ''
    if any(x.lower() in (c+' '+e['ObjectName']).lower() for x in pat):
        raw=e['$type'].split(',')[0].split('.')[-1]=='RawExport'
        print(f"== #{i} {e['ObjectName']} [{c}] outer={nm(e.get('OuterIndex',0))} {'RAW' if raw else ''}")
        for p in (e.get('Data') or []): print('    ',p['Name'],'=',json.dumps(pv(p))[:240])
