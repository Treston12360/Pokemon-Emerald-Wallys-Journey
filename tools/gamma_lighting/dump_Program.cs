using UAssetAPI.ExportTypes; using System; using System.IO; using UAssetAPI; using UAssetAPI.UnrealTypes; using UAssetAPI.Unversioned;
var map = new Usmap(args[0]);
var a = new UAsset(args[1], EngineVersion.VER_UE5_6, map);
int bad=0; foreach (var e in a.Exports) if (e is RawExport) bad++;
Console.Error.WriteLine($"exports={a.Exports.Count} raw(unparsed)={bad}");
File.WriteAllText(args[2], a.SerializeJson(true));
