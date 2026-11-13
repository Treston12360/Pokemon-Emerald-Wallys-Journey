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

## Exact material math (decoded from the game's compiled shaders)
Method: shaders come from ShaderArchive-PokemonEmerald-PCD3D_SM5 (Oodle-compressed DXBC, decompressed with
pyooz, disassembled with D3DDisassemble under Wine). Each material's cooked data names its shader map
(SHA hash) and holds a "preshader" program that fills the Material constant buffer (cb2) from its
parameters; decoded with docs-side tool notes below, parameter names resolved through the frozen name
patch table. Every number below is read, none estimated.

### Light rays (M_GodRay_MASTER, instance M_GodRay_Animated; Additive, Unlit, two-sided)
Values: Color (1.0, 0.845, 0.460) x Brightness 1.0; FadeDistance 512; Random 1.032; Width 361.8;
Length 799 (Width/Length shape the ray mesh in the vertex shader: SplineThicken).
Pixel math (per pixel):
  noise  = NoiseTex.r at (uv.x, uv.y*0.01 + (Time + Random)*0.1)            [T_TilingNoise02_M, scrolls up]
  mask   = MaskTex.r at uv                                                   [T_GodRay_Mask01]
  soft   = saturate((SceneDepth - PixelDepth) * 0.0025) * mask              [fades where it meets geometry, 400 units]
  N      = surface normal; V = direction to camera
  edge   = (1 - |N.V|^10)^8   (0 when |N.V| is ~0) ...  rays vanish when seen edge-on
  near   = 1 - saturate((1 - PixelDepth/FadeDistance) * 1.3333)            [fades out within 512 of the camera]
  alpha  = saturate(edge * soft * near * noise)
  out.rgb = max(Color*Brightness, 0) * alpha * fog transmittance, added to the screen.

### Cloud shadows (M_SunCloudShadows_LF1, light function on the sun)
Buffer: speedA = (SpeedY, SpeedX) * WindSpeed * -0.003 = (-0.0021, -0.0030)
        speedB = (SpeedX, SpeedY) * -0.002 = (-0.0010, -0.0007)
  P  = position (for a directional light function: world position in light space / LightFunctionScale;
       the sun keeps the engine default scale)
  n  = DistortionTex.r at (P.xy / 888.25 + Time * speedB)                    [parent default: T_NoiseRough]
  c  = CloudTex.r      at ((P.xy + n*40) / 5000 + Time * speedA)          [instance: T_NoisePuddles]
  light = 1 - saturate((c - 0.164) * 3.8417) * 0.75     (1 = full sun, 0.25 = darkest shadow)
  then blended toward 1 by the engine's light-function fade with distance; output sqrt(mean(rgb)).

### People / sprites (BR_DefaultLitSpriteMaterial; Masked, two-sided, Default Lit)
  base     = saturate(SpriteTex.rgb * vertexColor.rgb)
  discard if SpriteTex.a * vertexColor.a < 0.3333
  emissive = SpriteTex.rgb * base * Emissive  (Emissive = 0 -> none)
  Metallic 0, Roughness 1.0, Specular engine default 0.5, normal = flat normal map (sprite plane).
So people are plain matte surfaces lit by the same sun and sky light as the world.

## Tools note
Parser: UAssetAPI built from source (+ Newtonsoft.Json, stub zstd), Longvinter 5.7.2 mappings with the
removals listed at the top. Lives only in the session workspace (/home/claude/ue), not in the repo.

### Global state Gamma runs in (MPC_GlobalEnvironment, set by the sky blueprint)
Material Parameter Collection layout (cb1): [0]=(WindIntensity 1.2, WindAngle 0.2, WindSpeed 1, SwaySpeed),
[1]=(SwayDamping, SwayLean, DayLength, NightLength), [2]=(DayStartTime, CurrentTime, DayCycleProgress,
OrbitOrientation), [3]=(OrbitInclination, Overcast, RainStrength, RainWetness), [4]=(RainPuddles,
WeatherWindMult, WeatherAtmosphereMix, WeatherCloudFadeout), vectors: SunLightRotation, SunLightDirection,
MoonLightRotation, MoonLightDirection, OvercastColor (0.485,0.571,1.0), ...
All sky/fog shaders use phase t = frac(DayCycleProgress) and blend Day -> Sunset -> Night -> Sunrise -> Day
across t = 0..1 in quarters (p0 = sat(4t), p1 = sat(4t-1), p2 = sat(4t-2), p3 = max(4t-3,0)).
Gamma's level stores DayCycleProgress = 1.0 -> t = 0 -> pure DAY everywhere; weather Clear -> mix 0, overcast 0.

### Sky dome (M_StylizedSky + MI_StylizedSky_Dreamy) - day result
  h  = dome UV.y; noise = HorizonNoise.r at uv*(2, 8)
  h' = h + noise * 0.03 * saturate((h - 0.6) / 0.3)
  sky = SkyAtlas[row 15 (Dreamy day)] at x = 1 - h'   (gradient: docs/gamma/sky_day_gradient_256.csv,
        256 RGB values read from the baked atlas; 2.0 white at the horizon band, ~(0.64,0.82,0.81) at 0.25,
        (0.03,0.34,0.49) at 0.4, deep blue (0.0,0.115,0.573) at the top)
  * Sky Brightness 1.0; stars/nebula off by day (Star Brightness Day 0).
  + background clouds: BGCloud tex scrolled (speed -0.0004), tint (0.368,0.537,1.0) * strength 0.5, saturated.
  Saturation 1 (no change), overcast 0.

### Distance haze (M_StylizedFogPP post-process + MI_StylizedFogPP_Classic) - day result
  d    = scene depth;  f = saturate((d - 10) / (40000 - 10))
  dir  = normalized (pixel - camera);  h = min((1 - dir.z)^4, 1)   (= 1 for any downward view)
  amt  = saturate(f^1.0 * 0.5) * h
  sun  = saturate(1 - 0.4 * (dot(SunDir, normalize(camera - pixel)) - 0.5))      (Sunward 0.8)
  amt  = min(sun, amt)
  out  = lerp(scene, (0.2, 0.467, 1.0), amt)           (hue shift 0)
       + 3D fog: two 3D-noise samples at camera - dir*3000 and *5700, scale 1/12000 and 1.9/12000,
         drifting with wind (0.03*Time along the wind angle), mixed by 0.25, * strength * colour
         (0.120,0.198,0.356); strength from the current weather asset; Clear weather leaves it at 0 -> NO drifting fog layer in Gamma
       + sun glow: SunMoon atlas row 2 at noon = (0,0,0) -> no glow at Gamma's fixed time; moon glow 0 by day.

### Clouds on the dome (MI_StylizedClouds_Classic) - parameters (shader math not needed for our top-down view)
4 layers: speeds 2.0/1.4/1.4/1.4 x -0.0004, vertical stretch 3.33, offsets -0.141/-0.208/-0.18/-0.08,
strength 1.2 (layer 2: 1.5, layer 4: 1.3), flow offsets 30/100/60, highlight threshold 0.6 falloff 3 strength 4,
noise speed 0.7 strength 3; colour from Atlas_Clouds row 0 by day (0.12,0.30,0.64) -> (0.68,0.78,1.0).

### Runtime values from the sky blueprint's script (decoded Kismet bytecode of BP_StylizedSky)
- Sun direction: OrbitOrientator yaw = Orbit Orientation + 90 = 76.82; OrbitInclinator roll = Orbit Inclination 50;
  the light's own rotation = (pitch lerp(170,370,a), roll lerp(50,-50,a)) with day-timeline value a = 0.5
  (stored in the level) -> pitch 270 (= -90), roll 0. Composed, the light travels along
  (0.746, -0.175, -0.643) in Unreal axes; direction TO the sun = (-0.746, 0.175, 0.643):
  elevation exactly 40 degrees (90 - inclination).
- Player camera (BP_Brendan): spring arm 500 long, pitch -53.4, yaw 0, not inheriting the player's rotation,
  FOV 100 -> the screen's "up" is Unreal +X. So the sun sits behind the viewer (toward the bottom of the
  screen) and slightly to the right, 40 degrees up; shadows fall up-screen and slightly left.
- Sun intensity = 1 (brightness alpha) x Sun Light Brightness 3 x multiplier 1 = 3.0.
  Sun colour = SunColorCurve at CurrentTime/DayLength = 360/720 = 0.5 -> (0.788, 0.738, 0.700) linear
  (matches the stored 230,223,218).
- Sky light each update: intensity = Day Skylight Intensity 0.32 x weather multiplier 1.0;
  colour = Day Skylight Color (1.0, 0.701, 0.271) (overcast 0, no lightning flash).
- Height fog each update: density = Day Volumetric Density 0.02, albedo = Day Volumetric Color
  (1.0, 0.596, 0.090), height falloff 0.3 (weather mix 0).
- 3D fog strength comes from the weather asset; DA_Weather_Clear (and the class default) = 0.
