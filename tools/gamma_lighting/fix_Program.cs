using System; using System.Linq; using System.Collections.Generic; using System.Collections.Concurrent;
using UAssetAPI; using UAssetAPI.UnrealTypes; using UAssetAPI.Unversioned; using UAssetAPI.ExportTypes;
// usage: fix new.usmap old.usmap asset maxk class1 class2 ...
var nm = new Usmap(args[0]); var om = new Usmap(args[1]); string asset=args[2]; int maxk=int.Parse(args[3]);
var classes = args.Skip(4).ToArray();
var orig = classes.ToDictionary(c=>c, c=>nm.Schemas[c]);
var cand = new List<(string c,string p)>();
foreach (var c in classes) { var olds = om.Schemas.TryGetValue(c, out var o) ? o.Properties.Values.Select(x=>x.Name).ToHashSet() : new HashSet<string>();
  foreach (var p in nm.Schemas[c].Properties.OrderBy(k=>k.Key)) if (!olds.Contains(p.Value.Name)) cand.Add((c,p.Value.Name)); }
Console.WriteLine("candidates: " + string.Join(", ", cand.Select(x=>x.c+"."+x.p)));
void Apply(HashSet<(string,string)> rem) {
  foreach (var c in classes) { var s = orig[c]; var props = new ConcurrentDictionary<int,UsmapProperty>(); int i=0;
    foreach (var kv in s.Properties.OrderBy(k=>k.Key)) { if (rem.Contains((c,kv.Value.Name))) continue; var p=(UsmapProperty)kv.Value.Clone(); p.SchemaIndex=i; props[i]=p; i++; }
    nm.Schemas[c] = new UsmapSchema(s.Name, s.SuperType, i, props, false, s.SuperTypeModulePath, nm); nm.Schemas[c].ModulePath = s.ModulePath; nm.Schemas[c].StructKind=s.StructKind; }
}
int Try(HashSet<(string,string)> rem){ Apply(rem); try { var a=new UAsset(asset, EngineVersion.VER_UE5_6, nm); return a.Exports.Count(e=>e is RawExport);} catch { return 99999; } }
var fixedRem = (Environment.GetEnvironmentVariable("REMOVE")??"").Split(';',StringSplitOptions.RemoveEmptyEntries).Select(x=>{var q=x.Split('.');return (q[0],q[1]);}).ToHashSet();
if (Environment.GetEnvironmentVariable("OUT") is string outp) { Apply(fixedRem); var a=new UAsset(asset, EngineVersion.VER_UE5_6, nm); Console.WriteLine("raw="+a.Exports.Count(e=>e is RawExport)); System.IO.File.WriteAllText(outp,a.SerializeJson(true)); return; }
int baseRaw = Try(new()); Console.WriteLine($"base raw={baseRaw}");
int best=baseRaw;
for (int k=1;k<=Math.Min(maxk,cand.Count);k++){
  foreach (var combo in Combos(cand.Count,k)) { var rem = combo.Select(i=>cand[i]).ToHashSet(); int r=Try(rem);
    if (r<best || r==0) { best=Math.Min(best,r); Console.WriteLine($"k={k} raw={r} remove: "+string.Join(", ",rem.Select(x=>x.Item1+"."+x.Item2))); } }
}
static IEnumerable<int[]> Combos(int n,int k){ var a=Enumerable.Range(0,k).ToArray(); while(true){ yield return (int[])a.Clone(); int i=k-1; while(i>=0 && a[i]==n-k+i) i--; if(i<0) yield break; a[i]++; for(int j=i+1;j<k;j++) a[j]=a[j-1]+1; } }
