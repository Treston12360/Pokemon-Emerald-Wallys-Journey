import json, os, subprocess, shutil
S=os.environ.get('WS_DIR','build/ws_mockup')
R=os.environ.get('WJ_REPO','.')
scenes=[("MAP_RUSTBORO_CITY",20,28,40,60),("MAP_PETALBURG_CITY",15,16,30,30),("MAP_FORTREE_CITY",20,11,40,20),("MAP_OLDALE_TOWN_POKEMON_CENTER_1F",7,5,14,9),("MAP_RUSTBORO_CITY",20,28,40,60)]
epochs=[948369600,947592000,947678400,946900800]
env=dict(os.environ); env['PATH']='/opt/xpack/xpack-arm-none-eabi-gcc-14.2.1-1.1/bin:'+env['PATH']
meta={}
for dx in (-8,-4,0,4,8):
  for dy in (-4,0,4):
    tag='%+d_%+d'%(dx,dy)
    sc=[]
    for i,(m,x,y,w,h) in enumerate(scenes):
      nx=min(max(x+dx,0),w-1); ny=min(max(y+dy,0),h-1)
      sc.append({"name":"ws%d"%i,"map":m,"x":nx,"y":ny,"flags":[],"party":[]})
      meta.setdefault(i,{})[tag]=(nx-x,ny-y)
    json.dump(sc,open(S+'/t.json','w'))
    subprocess.run(['python3','tools/wj_testfw/install.py','on',S+'/t.json'],cwd=R,check=True,stdout=subprocess.DEVNULL)
    r=subprocess.run('make -j4',shell=True,cwd=R,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    subprocess.run(['python3','tools/wj_testfw/install.py','off'],cwd=R,check=True,stdout=subprocess.DEVNULL)
    assert r.returncode==0, tag
    shutil.copy(R+'/pokeemerald.gba',S+'/rom.gba')
    ps=[]
    for i,e in enumerate(epochs):
      d=S+'/shots/%d/%s'%(i,tag); os.makedirs(d,exist_ok=True)
      ps.append(subprocess.Popen('timeout 300 '+os.environ.get('WJ_HARNESS','harness')+' %s/rom.gba %s/s.txt %d > log 2>&1'%(S,S,e),shell=True,cwd=d))
    for p in ps: p.wait()
    print(tag,flush=True)
json.dump(meta,open(S+'/meta.json','w'))
