import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exp_curve_sim as E
def gaps(**kw):
    team,log,_=E.simulate(**kw)
    imp=['TRAINER_ROXANNE_1','TRAINER_BRAWLY_1','TRAINER_WALLY_MAUVILLE','TRAINER_WATTSON_1','TRAINER_WALLY_VERDANTURF','TRAINER_MAXIE_MT_CHIMNEY','TRAINER_FLANNERY_1','TRAINER_WALLY_LAVARIDGE','TRAINER_NORMAN_1','TRAINER_SHELLY_WEATHER_INSTITUTE','TRAINER_WINONA_1','TRAINER_WALLY_FORTREE','TRAINER_WALLY_LILYCOVE','TRAINER_TABITHA_MAGMA_HIDEOUT','TRAINER_MAXIE_MAGMA_HIDEOUT','TRAINER_MATT','TRAINER_TATE_AND_LIZA_1','TRAINER_WALLY_MOSSDEEP','TRAINER_ARCHIE','TRAINER_WALLY_SOOTOPOLIS','TRAINER_JUAN_1','TRAINER_WALLY_VR_1']
    prev=None; rows=[]
    for i,(m,t,l) in enumerate(log):
        if t in imp:
            before=log[i-1][2] if i else l
            p=[x[1] for x in E.parties[t]]
            a=sum(before)/len(before); b=sum(p)/len(p)
            rows.append((t[8:],a,b,max(p)))
    lv=log[-1][2]; rows.append(('SIDNEY',sum(lv)/6,51.8,53))
    return rows
if __name__=='__main__':
    print(f"{'fight':28s}{'party':>7s}{'foe avg':>9s}{'ace':>5s}{'gap':>6s}")
    for t,a,b,ace in gaps(): print(f"{t:28s}{a:7.1f}{b:9.1f}{ace:5d}{a-b:6.1f}")
