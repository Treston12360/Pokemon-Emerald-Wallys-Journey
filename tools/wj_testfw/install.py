#!/usr/bin/env python3
"""Install/remove TEST-ONLY hooks. usage: install.py on scenarios.json | install.py off"""
import sys, json, shutil, os
T=os.path.dirname(os.path.abspath(__file__)); R=os.path.dirname(os.path.dirname(T))
KEEP=T+'/orig'
files=['src/new_game.c','src/battle_setup.c']
def on(scn):
    os.makedirs(KEEP,exist_ok=True)
    for f in files: shutil.copy(R+'/'+f, KEEP+'/'+os.path.basename(f))
    shutil.copy(T+'/wj_test.c', R+'/src/wj_test.c')
    rows=[]
    for s in scn:
        fl=', '.join(s.get('flags',[]))+(', ' if s.get('flags') else '')+'0'
        vr=', '.join('{%s, %s}'%(a,b) for a,b in s.get('vars',[]))
        pt=', '.join('{SPECIES_%s, %d}'%(a,b) for a,b in s.get('party',[('MEWTWO',100),('MEW',100)]))
        rows.append('    {MAP_GROUP(%s), MAP_NUM(%s), %d, %d, {%s}, {%s}, {%s}}'%(s['map'],s['map'],s['x'],s['y'],fl,vr or '{0,0}',pt))
    open(R+'/src/wj_test_scenarios.h','w').write(('#define WJ_WEAK 1\n' if any(s.get('weak') for s in scn) else '')+('#define WJ_DEXDEMO 1\n' if any(s.get('dexdemo') for s in scn) else '')+'#include "constants/opponents.h"\n#include "constants/trainers.h"\nstatic const struct WJTestScenario sWJTestScenarios[] = {\n'+',\n'.join(rows)+'\n};\n')
    s=open(R+'/src/new_game.c').read()
    a='        SetWarpDestination(MAP_GROUP(MAP_INSIDE_OF_TRUCK), MAP_NUM(MAP_INSIDE_OF_TRUCK), WARP_ID_NONE, -1, -1);'
    assert a in s; s=s.replace(a,'        { void WJTest_Warp(void); WJTest_Warp(); }',1)
    a='    WJ_OutfitOnNewGame();\n}'
    assert a in s; s=s.replace(a,'    WJ_OutfitOnNewGame();\n    { void WJTest_ApplyState(void); WJTest_ApplyState(); }\n}',1)
    open(R+'/src/new_game.c','w').write(s)
    s=open(R+'/src/battle_setup.c').read()
    a='    CreateBattleStartTask(GetTrainerBattleTransition(), 0);\n    IncrementGameStat(GAME_STAT_TOTAL_BATTLES);\n    IncrementGameStat(GAME_STAT_TRAINER_BATTLES);'
    assert a in s; s=s.replace(a,'    { void WJTest_RecordEnemy(void); WJTest_RecordEnemy(); }\n'+a,1)
    open(R+'/src/battle_setup.c','w').write(s)
def off():
    for f in files: shutil.copy(KEEP+'/'+os.path.basename(f), R+'/'+f)
    for f in ('src/wj_test.c','src/wj_test_scenarios.h'):
        if os.path.exists(R+'/'+f): os.remove(R+'/'+f)
if sys.argv[1]=='on': on(json.load(open(sys.argv[2])))
else: off()
