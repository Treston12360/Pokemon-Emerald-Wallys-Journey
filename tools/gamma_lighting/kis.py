import json,sys
d=json.load(open(sys.argv[1])); im=d['Imports']; ex=d['Exports']
def ref(i):
    if isinstance(i,int):
        return im[-i-1]['ObjectName'] if i<0 else (ex[i-1]['ObjectName'] if i>0 else 'None')
    return str(i)
def pp(p):
    if isinstance(p,dict):
        if 'Path' in p and isinstance(p['Path'],list): return '.'.join(str(x) for x in p['Path'])
        if 'New' in p: return pp(p['New'])
        if 'Old' in p: return ref(p['Old'])
        if 'ResolvedOwner' in p: return pp(p.get('Path'))
    return str(p)
def E(e):
    if e is None: return ''
    if isinstance(e,list): return ', '.join(E(x) for x in e)
    if not isinstance(e,dict): return str(e)
    t=e.get('$type','').split(',')[0].split('.')[-1].replace('EX_','')
    g=lambda k:e.get(k)
    if t in('LocalVariable','InstanceVariable','LocalOutVariable','DefaultVariable'): return pp(g('Variable'))
    if t in('FloatConst','DoubleConst','IntConst','ByteConst','IntConstByte'): return str(g('Value'))
    if t=='True': return 'true'
    if t=='False': return 'false'
    if t in('NameConst',): return repr(str(g('Value')))
    if t in('StringConst','UnicodeStringConst'): return repr(g('Value'))
    if t=='ObjectConst': return ref(g('Value'))
    if t in('VectorConst','RotationConst','Vector3fConst'): return json.dumps({k:v for k,v in g('Value').items() if k!='$type'}) if isinstance(g('Value'),dict) else str(e)
    if t in('Let','LetObj','LetBool','LetWeakObjPtr','LetValueOnPersistentFrame','LetDelegate','LetMulticastDelegate'):
        return f"{E(g('Variable') or g('VariableExpression'))} = {E(g('Expression') or g('AssignmentExpression'))}"
    if t in('CallMath','FinalFunction','LocalFinalFunction','VirtualFunction','LocalVirtualFunction'):
        fn=ref(g('StackNode')) if g('StackNode') is not None else str(g('VirtualFunctionName'))
        return f"{fn}({E(g('Parameters'))})"
    if t=='Context' or t=='Context_FailSilent': return f"{E(g('ObjectExpression'))}.{E(g('ContextExpression'))}"
    if t=='StructMemberContext': return f"{E(g('StructExpression'))}.{pp(g('StructMemberExpression'))}"
    if t=='JumpIfNot': return f"if not ({E(g('BooleanExpression'))}) goto {g('CodeOffset')}"
    if t=='Jump': return f"goto {g('CodeOffset')}"
    if t=='Return': return f"return {E(g('ReturnExpression'))}"
    if t=='Self': return 'self'
    if t in('StructConst',): return f"{ref(g('Struct'))}{{{E(g('Value'))}}}"
    if t in('ArrayConst','SetArray'): return f"[{E(g('Elements'))}]"
    if t in('Cast','DynamicCast','MetaCast','PrimitiveCast','ObjToInterfaceCast'): return f"cast({E(g('Target'))})"
    if t in('ComputedJump','PopExecutionFlow','PushExecutionFlow','PopExecutionFlowIfNot','Nothing','EndOfScript','Tracepoint','WireTracepoint','InstrumentationEvent'): return t+(' '+E(g('BooleanExpression')) if g('BooleanExpression') else '')+(f" {g('PushingAddress')}" if g('PushingAddress') is not None else '')
    if t=='SwitchValue': return f"switch({E(g('IndexTerm'))})"
    return t+'?'
names=sys.argv[2:]
for e in ex:
    if e['ObjectName'] in names:
        print('=====',e['ObjectName'])
        for st in e.get('ScriptBytecode') or []:
            print('  ',E(st)[:400])
