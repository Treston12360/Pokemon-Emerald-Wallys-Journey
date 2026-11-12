import json,glob,os
def keys(o,acc):
    if isinstance(o,dict):
        if o.get('$type','').startswith('UAssetAPI.UnrealTypes.FRichCurveKey'): acc.append((o['Time'],o['Value'])); return
        for v in o.values(): keys(v,acc)
    elif isinstance(o,list):
        for v in o: keys(v,acc)
fl=lambda x: float(x) if not isinstance(x,str) else float(x.replace('+',''))
for f in sorted(glob.glob('/home/claude/ue/out/curves/Curve_*.json')):
    d=json.load(open(f)); print(os.path.basename(f)[:-5])
    for p in d['Exports'][0]['Data']:
        if p['Name']=='FloatCurves':
            a=[];keys(p,a); print('  '+'RGBA'[p['ArrayIndex']]+': '+' '.join(f"{fl(t):g}->{fl(v):.3f}" for t,v in a))
