# Gamma Emerald lighting values (for DLX-9)

Read 2026-11-12 from Gamma Emerald's own files (given to the owner by its developer). Engine: UE 5.6.
These are settings (numbers), not assets. We recreate the look in our OpenGL 3D view; nothing is copied.

How they were read: UAssetAPI (built from source, NuGet is blocked here) with the public Longvinter UE 5.7.2
mappings, corrected to 5.6 by removing `LightComponent.PreviewShadowMapChannel`, `LightComponent.DiffuseScale`
and `MaterialInstance.UserSceneTextureOverrides` (each verified: the exports then parse exactly).
Post-process: also remove `PostProcessSettings.bOverride_BloomGaussianIntensity`, `...BloomConvolutionIntensity`
(both the override and the value) and `PostProcessVolume.VolumeGuid` / `PostProcessComponent.VolumeGuid`;
verified by tracing every property read (values line up with their override flags).

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

## Screen look (post-process) - only settings with their override switched on are used
- Player camera (BP_Brendan): field of view 100. Colour saturation 1.25 (all channels).
  Depth of field on: focus distance 450, f-stop 1.74, sensor width 450, aspect 0.60 -> strong background
  blur (tilt-shift "miniature" look). (Camera also stores colour gain 1.5 and exposure 2.0, but their
  overrides are off, so they are not used.)
- Global post-process volume (unbound, priority 1): colour saturation R1.20 G1.19 B1.12 (vivid, slightly warm);
  auto exposure locked: min = max brightness 1.0 (no eye adaptation); motion blur 0 (off).
  Its blendables are the wild-battle transition and speed-line effects, not lighting.
- Sky actor post-process: the stylized fog material above (MI_StylizedFogPP_Classic), weight 1.
- Project settings: anti-aliasing off (crisp pixels), post-process quality 2, shadow quality 3.

## Town lights
- Littleroot: two spot lights (moving truck), intensity 4 candela, colour 255,241,141, cone 8-36 degrees.
- Dewford: street lamps and a campfire (point lights; brightness lives in the lamp blueprints, not read yet).

## Caves and dark rooms (BP_CaveLightTrigger, BP_BrawlyLightTrigger)
- Walking into the trigger box blends in a post-process: colour gain 0.49 (screen at ~half brightness)
  and vignette (dark edges) 3.0 in caves, 2.0 in Brawly's gym. Sharpen override on (default value).

## Lamps and indoor light
- Street lamp: point light 128 above the lamp base, intensity 2500 (default units), radius 771,
  colour 255,181,112 (warm orange), no shadows.
- Campfire: point light 45 up, intensity 2 candela, radius 379, colour 255,160,0.
- Interior window light (rect light): 512 x 336, intensity 125 candela, colour 255,218,142 (same warm
  tone as the outdoor fill light).
- Fake glow cards (no real light, a glowing sprite): colour 1.0,0.61,0.0, opacity 0.23, fades out
  near 0.24 and beyond 1024. Glowing variant: colour 1.0,0.64,0.24, glow 4.5.

## Sky colour curves (x = position from horizon (0) to top of sky (1); value = colour)
Dreamy sky, day:   R 0.18->2.0 0.24->0.67 0.35->0.11 0.49->0.02 0.70->0.03 1->0.0
                   G 0.18->2.0 0.24->0.85 0.35->0.45 0.49->0.21 0.70->0.13 1->0.11
                   B 0.18->2.0 0.24->0.84 0.35->0.50 0.49->0.57 0.70->0.63 1->0.57
  (white glow at the horizon, pale cyan band, deep blue overhead)
Dreamy sunset:     R 0->0.91 0.10->25 0.13->4.0 0.26->0.16 0.35->0.0 1->0.04
                   G 0->0.70 0.10->0.0 0.13->0.75 0.26->0.02 0.35->0.03 1->0.05
                   B 0->0.47 0.10->0.0 0.13->0.0 0.26->0.42 0.35->0.69 1->0.13
Dreamy night:      R 0.06->0.49 0.20->0.08 0.27->0.02 1->0.02; G 0.57,0.11,0.05,0.02; B 0.65,0.17,0.11,0.03
Dreamy sunrise:    R 0.05->5.0 0.17->0.96 0.23->0.60 0.35->0.06 0.62->0.04 1->0.06
                   G 1.58,0.14,0.60,0.39,0.36,0.09; B 0.90,0.0,0.0,0.21,0.51,0.22 (same x positions)
Dreamy moonset:    R 0.19->1.0 0.29->0.06 0.37->0.01 0.64->0.04; G 1.0,0.31,0.15,0.13; B 1.0,0.35,0.34,0.27
Clouds day (x = height in cloud): R 0->0.12 0.11->0.24 0.99->0.68; G 0.30,0.42,0.78; B 0.64,0.65,1.0
Clouds sunset: R 0.07,0.36,0.96; G 0.06,0.11,0.62; B 0.17,0.20,0.61 (x 0, 0.20, 1)
Clouds night:  R 0.01,0.03,0.07; G 0.03,0.09,0.24; B 0.06,0.19,0.50 (x 0, 0.23, 1)
Clouds sunrise: R 0.10,0.25,0.98; G 0.09,0.16,0.60; B 0.19,0.14,0.34 (x 0.05, 0.19, 1)
Sun colour over the day (x = time of day 0..1): R 0.02->0.88 0.20->0.82 0.60->0.78 0.91->0.97 0.98->2.0
  G 0.82,0.64,0.77,0.51,0.35; B 0.26,0.52,0.76,0.27,0.30 (warm at both ends, near white at midday).
Sun glow curve and moon colour curve: also read (in the parsed files; see tools note).

## Not read (and not needed to match the look)
- God-ray planes in Littleroot/Dewford: their values are inside a base material's node graph, not as
  settings. Recreate by eye (soft animated light shafts).
- Lit sprite material for people: base material graph; behaviour = sprites take the sun and fill light.

## Tools note
Parser: UAssetAPI built from source (+ Newtonsoft.Json, stub zstd), Longvinter 5.7.2 mappings with the
removals listed at the top. Lives only in the session workspace (/home/claude/ue), not in the repo.
