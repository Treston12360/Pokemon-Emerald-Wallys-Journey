# Gamma Emerald lighting values (for DLX-9)

Read 2026-11-12 from Gamma Emerald's own files (given to the owner by its developer). Engine: UE 5.6.
These are settings (numbers), not assets. We recreate the look in our OpenGL 3D view; nothing is copied.

How they were read: UAssetAPI (built from source, NuGet is blocked here) with the public Longvinter UE 5.7.2
mappings, corrected to 5.6 by removing `LightComponent.PreviewShadowMapChannel`, `LightComponent.DiffuseScale`
and `MaterialInstance.UserSceneTextureOverrides` (each verified: the exports then parse exactly).
Post-process volume settings (tone/bloom/LUT) did not parse yet (PostProcessSettings schema differs) - TODO.

## The setup (shared level MAP_Hoenn_Persistant, actor BP_StylizedSky, "SoStylized" sky kit)
- Time of day: Start/Current Time 360 (fixed at "day", Day Cycle Progress 1.0, IsDay true).
  Day Length 720, Night Length 216. Orbit Inclination 50, Orbit Orientation 346.8 (sun direction).
- Weather: static Clear. Planetary ring off.

## Sun (directional light)
- Intensity 3.0 ("Sun Light Brightness" 3.0), colour RGB 230,223,218 (slightly warm white).
- Soft shadows: LightSourceAngle 1.5, ShadowAmount 0.99, ShadowSharpen 0.75, shadow distance 20000.
- Light function = drifting cloud shadows (below).

## Sky light (ambient fill)
- Intensity 0.32, colour RGB 255,218,142 (warm). Day skylight colour (0..1): 1.0, 0.70, 0.27.
- Sunset: 1.0, 0.19, 0.03 @ 0.25. Night: 0.037, 0.10, 0.29 @ 0.81. Sunrise intensity 0.43.
- Separate static SkyLight actor: colour 100,129,126, occlusion tint 68,68,68.

## Fog
- Height fog density 0.02 (BP default 0.12), height falloff 0.3, max opacity 0.001 (very light).
- Volumetric: Day colour 1.0, 0.60, 0.09 (golden haze), density 0.02. 3D fog colour 0.12, 0.20, 0.36.
- Stylized fog post-process (MI_StylizedFogPP_Classic): Day atmosphere colour 0.20, 0.47, 1.0, strength 0.5,
  max distance 40000, height falloff 4. Sunset 0.0, 0.32, 1.0 @ 0.4. Night 0.068, 0.174, 0.458 @ 1.0.
  Sunrise 0.17, 0.17, 0.29 @ 0.4. Sun glow strength 5, falloff 7, hue shift -0.05, clamp 5. Sunward 0.8.

## Cloud shadows (MI_SunCloudShadows_LF1, light function on the sun)
- Noise texture, scale 5000, speed X 0.5 / Y 0.35, multiply 3.84, subtract 0.164, max opacity 0.75,
  distortion 40 at scale 888, wind speed 2.

## Sky (MI_StylizedSky_Dreamy)
- Brightness 1.0, horizontal noise 0.3 (x2, y8). Background clouds tint 0.37, 0.54, 1.0, strength 0.5.
- Clouds (MI_StylizedClouds_Classic): 4 layers, highlight strength 4, speed 2.
