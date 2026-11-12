# Tools used to read Gamma Emerald's files (DLX-9)

Everything here reproduces how `docs/GAMMA_LIGHTING_VALUES.md` was produced. Scratch tools, not part of the build.
Gamma's files: the owner's archive `Gamma-Pak-Recovery_part01..51.rar` (unpacked .pak, UE 5.6 "EA" build) and his
installed game `gamma-emerald-ea-windows` (has `PokemonEmerald-Win64-Shipping.exe`). The developer gave permission
to use them, textures included. List the archive with `7z l` (needs 7zip; unrar-free cannot), extract single files
with `7z x archive_part01.rar "path/*" -r`.

## Reading assets (.uasset/.umap + .uexp): UAssetAPI
- Clone github.com/atenfyr/UAssetAPI, JamesNK/Newtonsoft.Json. NuGet is blocked in this sandbox, so:
  retarget UAssetAPI to net8.0, remove PostSharp usings/[PSerializable], drop the ZstdSharp/Newtonsoft package refs
  and reference Newtonsoft's csproj instead (put it in its own ItemGroup, not the ADDTRACING one), add a stub
  `namespace ZstdSharp { class Decompressor {...} }`, fix two C# 14 lines (JmapHelper `res?.Type =`, Usmap
  IndentCharacter/IndentSize), put a nuget.config with `<clear/>` sources in the tree (and over nj/nuget.config,
  nj/Src/NuGet.Config), disable package validation + trim/AOT analyzers in Newtonsoft's csproj,
  build with `dotnet build -c Release -p:LibraryFrameworks=net8.0`.
- Mappings: Gamma's files are unversioned; use github.com/hihhhh1/longvinter-mapping (UE 5.7.2 usmap) and REMOVE
  these properties to get UE 5.6: `LightComponent.PreviewShadowMapChannel; LightComponent.DiffuseScale;
  MaterialInstance.UserSceneTextureOverrides; PostProcessSettings.bOverride_BloomGaussianIntensity;
  PostProcessSettings.BloomGaussianIntensity; PostProcessSettings.bOverride_BloomConvolutionIntensity;
  PostProcessSettings.BloomConvolutionIntensity; PostProcessVolume.VolumeGuid; PostProcessComponent.VolumeGuid`.
  `fix_Program.cs` applies removals (env REMOVE="Class.Prop;...", OUT=json) and can search removal sets;
  `fix2_Program.cs` traces each property read (patch MainSerializer.Read to print when UATRACE is set).
- `comp.py json Name,Name` prints exports; `mi.py` material instance params; `curve.py` curve keys;
  `kis.py json "Function"` prints Blueprint (Kismet) bytecode readably.

## Reading material math: compiled shaders
- `arch.py ShaderArchive-*.ushaderbytecode arch.pkl` indexes the archive; `find.py material.uexp` finds its shader
  map; `dump.py archive mapIndex outdir` decompresses (pip `pyooz`) and extracts DXBC; disassemble with
  `dis.c` (mingw-built, calls D3DDisassemble) under Wine.
- `preshader.py material.uexp` decodes the Material uniform-buffer program (which cb2 slot = which formula of
  which parameter) + parent default values; `pn2.py` resolves parameter names via the frozen name patch table.
- Engine shaders: ShaderArchive-Global-PCD3D_SM5 (eye adaptation = basic compute writing the exposure buffer,
  BloomSetup, CombineLUTs).

## Reading the program (.exe)
`xref.py "r.Some.CVar" [before after]` finds console-variable registrations and prints the default value load
(r8d for ints, xmm2 for floats). Needs `pip install capstone pefile`.

## Data
- `docs/gamma/gamma_clouds.bin`: T_NoisePuddles then T_NoiseRough, 1024x1024 8-bit each. Copy into the game's
  `voxel/` folder; `voxel_gl.c` loads it when `WJPC_3D_GAMMALIGHT=1`.
- `docs/gamma/sky_day_gradient_256.csv`: Gamma's day sky colours (baked atlas row 15).
- `gamma2d.py shot.ppm out.png pxPerTile`: the 2D preview (ground lit flat, Gamma clouds, bloom, film curve).
