using System; using System.Linq; using System.Collections.Generic; using System.Collections.Concurrent;
using UAssetAPI; using UAssetAPI.UnrealTypes; using UAssetAPI.Unversioned; using UAssetAPI.ExportTypes;
// fix2 new old asset targetNameSubstr classes...   (REMOVE env = fixed removals)
var nm = new Usmap(args[0]); var om = new Usmap(args[1]); string asset=args[2]; string tgt=args[3];
var classes = args.Skip(4).ToArray();
var fixedRem = (Environment.GetEnvironmentVariable("REMOVE")??"").Split(';',StringSplitOptions.RemoveEmptyEntries).Select(x=>{var q=x.Split('.');return (q[0],q[1]);}).ToHashSet();
var allCls = classes.Concat(fixedRem.Select(x=>x.Item1)).Distinct().ToArray();
var orig = allCls.ToDictionary(c=>c, c=>nm.Schemas[c]);
var cand = new List<(string c,string p)>();
foreach (var c in classes) { var olds = om.Schemas.TryGetValue(c, out var o) ? o.Properties.Values.Select(x=>x.Name).ToHashSet() : new HashSet<string>();
  foreach (var p in nm.Schemas[c].Properties.OrderBy(k=>k.Key)) if (!olds.Contains(p.Value.Name) && !cand.Contains((c,p.Value.Name))) cand.Add((c,p.Value.Name)); }
var groups = cand.GroupBy(x=>x.c+"."+x.p.Replace("bOverride_","").Replace("PetzvalBokehFalloff","PetzvalBokeh")).Select(g=>g.ToList()).ToList();
Console.WriteLine($"{groups.Count} groups: "+string.Join(" | ",groups.Select(g=>g[0].c+"."+g[0].p.Replace("bOverride_",""))));
void Apply(HashSet<(string,string)> rem) {
  foreach (var c in allCls) { var s = orig[c]; var props = new ConcurrentDictionary<int,UsmapProperty>(); int i=0;
    foreach (var kv in s.Properties.OrderBy(k=>k.Key)) { if (rem.Contains((c,kv.Value.Name))) continue; var p=(UsmapProperty)kv.Value.Clone(); p.SchemaIndex=i; props[i]=p; i++; }
    var ns = new UsmapSchema(s.Name, s.SuperType, i, props, false, s.SuperTypeModulePath, nm); ns.ModulePath = s.ModulePath; ns.StructKind=s.StructKind; nm.Schemas[c]=ns; }
}
// find target export indices once
var probe = new UAsset(asset, EngineVersion.VER_UE5_6, nm);
var targets = Enumerable.Range(0,probe.Exports.Count).Where(i=>probe.Exports[i].ObjectName.ToString().Contains(tgt)).ToArray();
Console.WriteLine("targets: "+string.Join(",",targets.Select(i=>probe.Exports[i].ObjectName.ToString())));
var skips = Enumerable.Range(0,probe.Exports.Count).Except(targets).ToArray();
long Prog(HashSet<(string,string)> rem){ var r=new HashSet<(string,string)>(rem); r.UnionWith(fixedRem); Apply(r); UAsset.FailPos.Clear();
  try { var a=new UAsset(EngineVersion.VER_UE5_6, nm); a.FilePath=asset; a.Read(a.PathToReader(asset), skips, null);
    long t=0; foreach(var i in targets) t += a.Exports[i] is RawExport ? Math.Min(UAsset.FailPos.GetValueOrDefault(i,0), a.Exports[i].SerialSize-1) : a.Exports[i].SerialSize*10; return t; } catch { return -1; } }
int Try(HashSet<(string,string)> rem)=> 0;
if (Environment.GetEnvironmentVariable("ONCE")!=null){ Console.Error.WriteLine("=== PROG"); Prog(new()); return; }
{ var cur=new HashSet<int>(); long best=Prog(new()); Console.WriteLine("start progress "+best);
  for(int pass=0;pass<4;pass++){ bool ch=false; for(int g=0;g<groups.Count;g++){ var t=new HashSet<int>(cur); if(!t.Remove(g)) t.Add(g);
      long p=Prog(t.SelectMany(i=>groups[i]).ToHashSet()); if(p>best){best=p;cur=t;ch=true; Console.WriteLine($"pass{pass} toggle {groups[g][0].p} -> {p} set: "+string.Join(", ",cur.Select(i=>groups[i][0].p.Replace("bOverride_",""))));} }
    if(!ch) break; }
  Console.WriteLine("FINAL remove: "+string.Join(";",cur.SelectMany(i=>groups[i]).Select(x=>x.c+"."+x.p))); return; }
var sw=System.Diagnostics.Stopwatch.StartNew(); int b=Try(new()); Console.WriteLine($"base={b} {sw.ElapsedMilliseconds}ms");
int n=groups.Count; int found=0;
for (int k=0;k<=n && found<20;k++) foreach (var combo in Combos(n,k)) {
  var rem = combo.SelectMany(i=>groups[i]).ToHashSet(); int r=Try(rem);
  if (r==0) { found++; Console.WriteLine($"OK k={k}: "+string.Join(", ",combo.Select(i=>groups[i][0].p.Replace("bOverride_","")))); if(found>=20) break; } }
static IEnumerable<int[]> Combos(int n,int k){ if(k==0){yield return new int[0]; yield break;} var a=Enumerable.Range(0,k).ToArray(); while(true){ yield return (int[])a.Clone(); int i=k-1; while(i>=0 && a[i]==n-k+i) i--; if(i<0) yield break; a[i]++; for(int j=i+1;j<k;j++) a[j]=a[j-1]+1; } }
